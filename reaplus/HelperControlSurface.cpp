#include "HelperControlSurface.h"

#include "FxParameter.h"
#include "TrackSend.h"
#include "Track.h"
#include "Project.h"
#include "Reaper.h"
#include "TrackVolume.h"
#include "TrackPan.h"
#include "TrackSendVolume.h"
#include "FxChain.h"
#include "reaper_plugin_functions.h"

using std::unique_lock;
namespace rx = rxcpp;
namespace rxsub = rxcpp::subjects;
using std::function;
using std::mutex;
using std::pair;
using std::string;
using std::set;


namespace reaplus {
  HelperControlSurface::HelperControlSurface() {
    reaper::plugin_register("csurf_inst", this);
  }

  rxcpp::subscription HelperControlSurface::enqueueCommand(function<void(void)> command) {
    rxcpp::composite_subscription subscription;
    const auto& worker = mainThreadCoordinator_.get_worker();
    worker.schedule(
        rxcpp::schedulers::make_schedulable(worker, subscription, [command](const rxcpp::schedulers::schedulable&) {
          command();
        })
    );
    return subscription;
  }

  const char* HelperControlSurface::GetTypeString() {
    return "";
  }

  const char* HelperControlSurface::GetDescString() {
    return "";
  }

  const char* HelperControlSurface::GetConfigString() {
    return "";
  }

  void HelperControlSurface::Run() {
    while (!mainThreadRunLoop_.empty() && mainThreadRunLoop_.peek().when <= mainThreadRunLoop_.now()) {
      mainThreadRunLoop_.dispatch();
    }
  }


  void HelperControlSurface::SetSurfaceVolume(MediaTrack* trackid, double volume) {
    Track track(trackid, nullptr);
    trackVolumeChangedSubject_.get_subscriber().on_next(track);
    if (!trackParameterIsAutomated(track, "Volume")) {
      trackVolumeTouchedSubject_.get_subscriber().on_next(track);
    }
  }


  void HelperControlSurface::SetSurfacePan(MediaTrack* trackid, double pan) {
    Track track(trackid, nullptr);
    trackPanChangedSubject_.get_subscriber().on_next(track);
    if (!trackParameterIsAutomated(track, "Pan")) {
      trackPanTouchedSubject_.get_subscriber().on_next(track);
    }
  }


  HelperControlSurface& HelperControlSurface::instance() {
    static HelperControlSurface INSTANCE;
    return INSTANCE;
  }

  rx::observable <FxParameter> HelperControlSurface::fxParameterValueChanged() const {
    return fxParameterValueChangedSubject_.get_observable();
  }

  void HelperControlSurface::SetTrackTitle(MediaTrack* trackid, const char* title) {
    if (state() == State::PropagatingTrackSetChanges) {
      numTrackSetChangesLeftToBePropagated_--;
    } else {
      trackNameChangedSubject_.get_subscriber().on_next(Track(trackid, nullptr));
    }
  }

  rxcpp::observable<Fx> HelperControlSurface::fxEnabledChanged() const {
    return fxEnabledChangedSubject_.get_observable();
  }

  int HelperControlSurface::Extended(int call, void* parm1, void* parm2, void* parm3) {
    switch (call) {
    case CSURF_EXT_SETINPUTMONITOR: {
      const auto mediaTrack = (MediaTrack*) parm1;
      trackInputChangedSubject_.get_subscriber().on_next(Track(mediaTrack, nullptr));
      return 0;
    }
    case CSURF_EXT_SETFXPARAM: {
      const auto mediaTrack = (MediaTrack*) parm1;
      const auto fxAndParamIndex = (int*) parm2;
      const int fxIndex = (*fxAndParamIndex >> 16) & 0xffff;
      const int paramIndex = *fxAndParamIndex & 0xffff;
      // Unfortunately, we don't have a ReaProject* here. Therefore we pass a nullptr.
      const Track track(mediaTrack, nullptr);
      const double paramValue = *(double*) parm3;
      // TODO In the ReaPlus integration test, there's one test where this heuristic doesn't work. Deal with it!
      const bool isInputFx = isProbablyInputFx(track, fxIndex, paramIndex, paramValue);
      const auto fxChain = isInputFx ? track.inputFxChain() : track.normalFxChain();
      if (const auto fx = fxChain.fxByIndex(fxIndex)) {
        const auto fxParam = fx->parameterByIndex(paramIndex);
        fxParameterValueChangedSubject_.get_subscriber().on_next(fxParam);
        if (fxHasBeenTouchedJustAMomentAgo_) {
          fxHasBeenTouchedJustAMomentAgo_ = false;
          fxParameterTouchedSubject_.get_subscriber().on_next(fxParam);
        }
      }
      return 0;
    }
    case CSURF_EXT_SETFXENABLED: {
      const auto mediaTrack = (MediaTrack*) parm1;
      const auto fxIndex = *(int*) parm2;
      // Unfortunately, we don't have a ReaProject* here. Therefore we pass a nullptr.
      const Track track(mediaTrack, nullptr);
      const bool isInputFx = isProbablyInputFx(track, fxIndex);
      const auto fxChain = isInputFx ? track.inputFxChain() : track.normalFxChain();
      if (const auto fx = fxChain.fxByIndex(fxIndex)) {
        fxEnabledChangedSubject_.get_subscriber().on_next(*fx);
      }
      return 0;
    }
    case CSURF_EXT_SETSENDVOLUME: {
      const auto mediaTrack = (MediaTrack*) parm1;
      const int sendIdx = *(int*) parm2;
      const Track track(mediaTrack, nullptr);
      const auto trackSend = track.indexBasedSendByIndex(sendIdx);
      trackSendVolumeChangedSubject_.get_subscriber().on_next(trackSend);

      // Send volume touch event only if not automated
      if (!trackParameterIsAutomated(track, "Send Volume")) {
        trackSendVolumeTouchedSubject_.get_subscriber().on_next(trackSend);
      }
      return 0;
    }
    case CSURF_EXT_SETFXCHANGE: {
      const auto mediaTrack = (MediaTrack*) parm1;
      detectFxChangesOnTrack(Track(mediaTrack, nullptr));
      return 0;
    }
    case CSURF_EXT_SETLASTTOUCHEDFX: {
      fxHasBeenTouchedJustAMomentAgo_ = true;
      return 0;
    }
    default:
      return 0;
    }

  }

  void HelperControlSurface::SetTrackListChange() {
    // FIXME Not multi-project compatible!
    numTrackSetChangesLeftToBePropagated_ = reaper::CountTracks(nullptr) + 1;
    removeInvalidReaProjects();
    detectTrackSetChanges();
  }

  HelperControlSurface::State HelperControlSurface::state() const {
    return numTrackSetChangesLeftToBePropagated_ == 0 ? State::Normal : State::PropagatingTrackSetChanges;
  }

  void HelperControlSurface::detectTrackSetChanges() {
    const auto project = Reaper::instance().currentProject();
    auto& oldMediaTracks = mediaTracksByReaProject_[project.reaProject()];
    const int oldTrackCount = (int) oldMediaTracks.size();
    const int newTrackCount = project.trackCount();
    if (newTrackCount < oldTrackCount) {
      removeInvalidMediaTracks(project, oldMediaTracks);
    } else if (newTrackCount > oldTrackCount) {
      addMissingMediaTracks(project, oldMediaTracks);
    } else {
      tracksReorderedSubject_.get_subscriber().on_next(project);
    }
  }

  void HelperControlSurface::addMissingMediaTracks(const Project& project,
      std::set<MediaTrack*>& mediaTracks) {
    project.tracks().subscribe([this, &mediaTracks](Track track) {
      const auto wasInserted = mediaTracks.insert(track.mediaTrack()).second;
      if (wasInserted) {
        trackAddedSubject_.get_subscriber().on_next(track);
        detectFxChangesOnTrack(track);
      }
    });
  }

  void HelperControlSurface::removeInvalidMediaTracks(const Project& project, std::set<MediaTrack*>& mediaTracks) {
    for (auto it = mediaTracks.begin(); it != mediaTracks.end();) {
      const auto mediaTrack = *it;
      if (reaper::ValidatePtr2(project.reaProject(), (void*) mediaTrack, "MediaTrack*")) {
        it++;
      } else {
        fxChainPairByMediaTrack_.erase(mediaTrack);
        trackRemovedSubject_.get_subscriber().on_next(Track(mediaTrack, project.reaProject()));
        it = mediaTracks.erase(it);
      }
    }
  }

  void HelperControlSurface::removeInvalidReaProjects() {
    for (auto it = mediaTracksByReaProject_.begin(); it != mediaTracksByReaProject_.end();) {
      const auto pair = *it;
      const auto project = pair.first;
      if (reaper::ValidatePtr2(nullptr, (void*) project, "ReaProject*")) {
        it++;
      } else {
        it = mediaTracksByReaProject_.erase(it);
      }
    }
  }

  rx::observable <Track> HelperControlSurface::trackRemoved() const {
    return trackRemovedSubject_.get_observable();
  }

  rx::observable <Track> HelperControlSurface::trackAdded() const {
    return trackAddedSubject_.get_observable();
  }

  rx::observable <Track> HelperControlSurface::trackVolumeChanged() const {
    return trackVolumeChangedSubject_.get_observable();
  }

  rx::observable <Track> HelperControlSurface::trackPanChanged() const {
    return trackPanChangedSubject_.get_observable();
  }

  rxcpp::observable<Track> HelperControlSurface::trackNameChanged() const {
    return trackNameChangedSubject_.get_observable();
  }

  rxcpp::observable<Track> HelperControlSurface::trackInputChanged() const {
    return trackInputChangedSubject_.get_observable();
  }

  rx::observable <TrackSend> HelperControlSurface::trackSendVolumeChanged() const {
    return trackSendVolumeChangedSubject_.get_observable();
  }

  const rxcpp::observe_on_one_worker& HelperControlSurface::mainThreadCoordination() const {
    return mainThreadCoordination_;
  }

  void HelperControlSurface::init() {
    HelperControlSurface::instance();
  }

  rx::observable <Track> HelperControlSurface::fxReordered() const {
    return fxReorderedSubject_.get_observable();
  }

  void HelperControlSurface::detectFxChangesOnTrack(Track track) {
    if (track.isAvailable()) {
      MediaTrack* mediaTrack = track.mediaTrack();
      auto& fxChainPair = fxChainPairByMediaTrack_[mediaTrack];
      bool addedOrRemovedOutputFx = detectFxChangesOnTrack(track, fxChainPair.outputFxGuids, false);
      bool addedOrRemovedInputFx = detectFxChangesOnTrack(track, fxChainPair.inputFxGuids, true);
      if (!addedOrRemovedInputFx && !addedOrRemovedOutputFx) {
        fxReorderedSubject_.get_subscriber().on_next(track);
      }
    }
  }


  bool HelperControlSurface::detectFxChangesOnTrack(Track track, set<string>& oldFxGuids, bool isInputFx) {
    const int oldFxCount = (int) oldFxGuids.size();
    const int newFxCount = (isInputFx ? track.inputFxChain() : track.normalFxChain()).fxCount();
    if (newFxCount < oldFxCount) {
      removeInvalidFx(track, oldFxGuids, isInputFx);
      return true;
    } else if (newFxCount > oldFxCount) {
      addMissingFx(track, oldFxGuids, isInputFx);
      return true;
    } else {
      // Reordering (or nothing)
      return false;
    }
  }


  void HelperControlSurface::removeInvalidFx(Track track, std::set<string>& oldFxGuids, bool isInputFx) {
    const auto newFxGuids = fxGuidsOnTrack(track, isInputFx);
    for (auto it = oldFxGuids.begin(); it != oldFxGuids.end();) {
      const string oldFxGuid = *it;
      if (newFxGuids.count(oldFxGuid)) {
        it++;
      } else {
        const auto fxChain = isInputFx ? track.inputFxChain() : track.normalFxChain();
        fxRemovedSubject_.get_subscriber().on_next(fxChain.fxByGuid(oldFxGuid));
        it = oldFxGuids.erase(it);
      }
    }
  }

  void HelperControlSurface::addMissingFx(Track track, std::set<string>& fxGuids, bool isInputFx) {
    const auto fxChain = isInputFx ? track.inputFxChain() : track.normalFxChain();
    fxChain.fxs().subscribe([this, &fxGuids](Fx fx) {
      bool wasInserted = fxGuids.insert(fx.guid()).second;
      if (wasInserted) {
        fxAddedSubject_.get_subscriber().on_next(fx);
      }
    });
  }

  std::set<string> HelperControlSurface::fxGuidsOnTrack(Track track, bool isInputFx) const {
    const auto fxChain = isInputFx ? track.inputFxChain() : track.normalFxChain();
    std::set<string> fxGuids;
    fxChain.fxs().subscribe([&fxGuids](Fx fx) {
      fxGuids.insert(fx.guid());
    });
    return fxGuids;
  }

  rx::observable <Fx> HelperControlSurface::fxAdded() const {
    return fxAddedSubject_.get_observable();
  }

  rx::observable <Fx> HelperControlSurface::fxRemoved() const {
    return fxRemovedSubject_.get_observable();
  }

  rx::observable <Project> HelperControlSurface::tracksReordered() const {
    return tracksReorderedSubject_.get_observable();
  }

  bool HelperControlSurface::isProbablyInputFx(Track track, int fxIndex, int paramIndex, double fxValue) const {
    const auto mediaTrack = track.mediaTrack();
    if (fxChainPairByMediaTrack_.count(mediaTrack)) {
      const auto& fxChainPair = fxChainPairByMediaTrack_.at(mediaTrack);
      const bool couldBeInputFx = fxIndex < fxChainPair.inputFxGuids.size();
      const bool couldBeOutputFx = fxIndex < fxChainPair.outputFxGuids.size();
      if (!couldBeInputFx && couldBeOutputFx) {
        return false;
      } else if (couldBeInputFx && !couldBeOutputFx) {
        return true;
      } else { // could be both
        if (paramIndex == -1) {
          // We don't have a parameter number at our disposal so we need to guess - we guess normal FX TODO
          return false;
        } else {
          // Compare parameter values (a heuristic but so what, it's just for MIDI learn)
          if (const auto outputFx = track.normalFxChain().fxByIndex(fxIndex)) {
            FxParameter outputFxParam = outputFx->parameterByIndex(paramIndex);
            bool isProbablyOutputFx = outputFxParam.reaperValue() == fxValue;
            return !isProbablyOutputFx;
          } else {
            return true;
          }
        }
      }
    } else {
      // Should not happen. In this case, an FX yet unknown to Realearn has sent a parameter change
      return false;
    }
  }

  bool HelperControlSurface::isProbablyInputFx(Track track, int fxIndex) const {
    return isProbablyInputFx(track, fxIndex, -1, -1);
  }

  rx::observable<Parameter*> HelperControlSurface::parameterValueChangedUnsafe() const {
    return fxParameterValueChanged().map([this](FxParameter fxParam) -> Parameter* {
          return new FxParameter(fxParam);
        })
        .merge(trackVolumeChanged().map([this](Track track) -> Parameter* {
          return new TrackVolume(track);
        }))
        .merge(trackPanChanged().map([this](Track track) -> Parameter* {
          return new TrackPan(track);
        }))
        .merge(trackSendVolumeChanged().map([this](TrackSend trackSend) -> Parameter* {
          return new TrackSendVolume(trackSend);
        }));
  }

  rx::observable<Parameter*> HelperControlSurface::parameterTouchedUnsafe() const {
    return fxParameterTouched().map([this](FxParameter fxParam) -> Parameter* {
          return new FxParameter(fxParam);
        })
        .merge(trackVolumeTouched().map([this](Track track) -> Parameter* {
          return new TrackVolume(track);
        }))
        .merge(trackPanTouched().map([this](Track track) -> Parameter* {
          return new TrackPan(track);
        }))
        .merge(trackSendVolumeTouched().map([this](TrackSend trackSend) -> Parameter* {
          return new TrackSendVolume(trackSend);
        }));
  }

  rx::observable <FxParameter> HelperControlSurface::fxParameterTouched() const {
    return fxParameterTouchedSubject_.get_observable();
  }

  rx::observable <Track> HelperControlSurface::trackVolumeTouched() const {
    return trackVolumeTouchedSubject_.get_observable();
  }

  rx::observable <Track> HelperControlSurface::trackPanTouched() const {
    return trackPanTouchedSubject_.get_observable();
  }

  rx::observable <TrackSend> HelperControlSurface::trackSendVolumeTouched() const {
    return trackSendVolumeTouchedSubject_.get_observable();
  }

  bool HelperControlSurface::trackParameterIsAutomated(Track track, string parameterName) const {
    if (track.isAvailable() && reaper::GetTrackEnvelopeByName(track.mediaTrack(), parameterName.c_str()) != nullptr) {
      // There's at least one automation lane for this parameter
      auto automationMode = track.effectiveAutomationMode();
      switch (automationMode) {
      case AutomationMode::Bypass:
      case AutomationMode::TrimRead:
      case AutomationMode::Write:
        // Is not automated
        return false;
      default:
        // Is automated
        return true;
      }
    } else {
      return false;
    }
  }


}