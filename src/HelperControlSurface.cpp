#include <reaplus/HelperControlSurface.h>
#include <utility>
#include <reaplus/TrackSend.h>
#include <reaplus/Reaper.h>
#include <reaplus/TrackVolume.h>
#include <reaplus/TrackPan.h>
#include <reaplus/TrackSendVolume.h>
#include <reaplus/MasterTempo.h>
#include <reaplus/MasterPlayrate.h>
#include <reaplus/TrackArm.h>
#include <reaplus/TrackMute.h>
#include <reaplus/TrackSolo.h>
#include <reaplus/TrackSelection.h>
#include <reaplus/TrackSendPan.h>
#include <reaplus/FxEnable.h>
#include <reaper_plugin_functions.h>
#include <reaplus/utility.h>
#include <reaplus/util/log.h>

using std::unique_lock;
namespace rx = rxcpp;
namespace rxsub = rxcpp::subjects;
using std::function;
using std::mutex;
using std::pair;
using std::string;
using std::set;
using std::unique_ptr;
using boost::none;
using reaplus::util::logException;

namespace reaplus {
  std::unique_ptr<HelperControlSurface> HelperControlSurface::INSTANCE = nullptr;

  HelperControlSurface::HelperControlSurface() :
      activeProjectBehavior_(Reaper::instance().currentProject()),
      fastCommandQueue_(1000) {
    // Detect features
    const string reaperVersion = reaper::GetAppVersion();
    supportsDetectionOfInputFx_ = reaperVersion >= "5.95"; // since pre1
    supportsDetectionOfInputFxInSetFxChange_ = reaperVersion >= "5.95"; // since pre2 to be accurate but so what

    // Register
    reaper::plugin_register("csurf_inst", this);

    // REAPER doesn't seem to call this automatically when the surface is registered. In our case it's important
    // to call this not at the first change of something (e.g. arm button pressed) but immediately. Because it
    // captures the initial project/track/FX state. If we don't do this immediately, then it happens that change
    // events (e.g. track arm changed) are not reported because the initial state was unknown.
    SetTrackListChange();
  }

  HelperControlSurface::~HelperControlSurface() {
    reaper::plugin_register("-csurf_inst", this);
  }

  rxcpp::composite_subscription HelperControlSurface::enqueueCommand(function<void(void)> command) {
    rxcpp::composite_subscription subscription;
    const auto& worker = mainThreadCoordinator_.get_worker();
    worker.schedule(
        rxcpp::schedulers::make_schedulable(
            worker,
            subscription,
            [command = std::move(command)](const rxcpp::schedulers::schedulable&) {
              command();
            }
        )
    );
    return subscription;
  }

  void HelperControlSurface::enqueueCommandFast(std::function<void(void)> command) {
    fastCommandQueue_.enqueue(std::move(command));
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
    try {
      // Invoke custom idle code
      mainThreadIdleSubject_.get_subscriber().on_next(true);
      // Process items from fast queue
      const auto count = fastCommandQueue_.try_dequeue_bulk(fastCommandBuffer_.begin(), FAST_COMMAND_BUFFER_SIZE);
      for (auto i = 0; i < count; i++) {
        fastCommandBuffer_.at(i)();
      }
      // Process items from slow queue
      const auto fixedNow = mainThreadRunLoop_.now();
      const auto maxExecutionTime = std::chrono::milliseconds(50);
      while (mainThreadRunLoop_.now() - fixedNow <= maxExecutionTime
          && !mainThreadRunLoop_.empty()
          && mainThreadRunLoop_.peek().when <= fixedNow) {
        mainThreadRunLoop_.dispatch();
      }
    } catch (...) {
      logException();
    }
  }

  void HelperControlSurface::SetSurfaceVolume(MediaTrack* trackid, double volume) {
    try {
      if (state() != State::PropagatingTrackSetChanges) {
        if (auto td = findTrackDataByTrack(trackid)) {
          if (td->volume != volume) {
            td->volume = volume;
            Track track(trackid, nullptr);
            trackVolumeChangedSubject_.get_subscriber().on_next(track);
            if (!trackParameterIsAutomated(track, "Volume")) {
              trackVolumeTouchedSubject_.get_subscriber().on_next(track);
            }
          }
        }
      }
    } catch (...) {
      logException();
    }
  }

  void HelperControlSurface::SetSurfacePan(MediaTrack* trackid, double pan) {
    try {
      if (state() != State::PropagatingTrackSetChanges) {
        if (auto td = findTrackDataByTrack(trackid)) {
          if (td->pan != pan) {
            td->pan = pan;
            Track track(trackid, nullptr);
            trackPanChangedSubject_.get_subscriber().on_next(track);
            if (!trackParameterIsAutomated(track, "Pan")) {
              trackPanTouchedSubject_.get_subscriber().on_next(track);
            }
          }
        }
      }
    } catch (...) {
      logException();
    }
  }

  HelperControlSurface& HelperControlSurface::instance() {
    static Guard guard;
    if (INSTANCE == nullptr) {
      INSTANCE = unique_ptr<HelperControlSurface>(new HelperControlSurface());
    }
    return *INSTANCE;
  }

  void HelperControlSurface::destroyInstance() {
    if (INSTANCE != nullptr) {
      INSTANCE = nullptr;
    }
  }

  HelperControlSurface::Guard::~Guard() {
    Reaper::destroyInstance();
  }

  rx::observable<FxParameter> HelperControlSurface::fxParameterValueChanged() const {
    return fxParameterValueChangedSubject_.get_observable();
  }

  void HelperControlSurface::SetTrackTitle(MediaTrack* trackid, const char*) {
    try {
      if (state() == State::PropagatingTrackSetChanges) {
        numTrackSetChangesLeftToBePropagated_--;
      } else {
        trackNameChangedSubject_.get_subscriber().on_next(Track(trackid, nullptr));
      }
    } catch (...) {
      logException();
    }
  }

  rxcpp::observable<Fx> HelperControlSurface::fxEnabledChanged() const {
    return fxEnabledChangedSubject_.get_observable();
  }

  rxcpp::observable<Fx> HelperControlSurface::fxEnabledTouched() const {
    return fxEnabledChanged();
  }

  boost::optional<Fx> HelperControlSurface::getFxFromParmFxIndex(const Track& track, int parmFxIndex,
      int paramIndex, int paramValue) const {
    if (supportsDetectionOfInputFx_) {
      return track.fxByQueryIndex(parmFxIndex);
    } else {
      const bool isInputFx = isProbablyInputFx(track, parmFxIndex, paramIndex, paramValue);
      const auto fxChain = isInputFx ? track.inputFxChain() : track.normalFxChain();
      return fxChain.fxByIndex(parmFxIndex);
    }
  }

  int HelperControlSurface::Extended(int call, void* parm1, void* parm2, void* parm3) {
    try {
      switch (call) {
        // DONE-rust
        case CSURF_EXT_SETINPUTMONITOR: {
          if (state() != State::PropagatingTrackSetChanges) {
            const auto mediaTrack = (MediaTrack*) parm1;
            if (auto td = findTrackDataByTrack(mediaTrack)) {
              {
                const auto recmonitor = (int*) parm2;
                if (td->recmonitor != *recmonitor) {
                  td->recmonitor = *recmonitor;
                  trackInputMonitoringChangedSubject_.get_subscriber().on_next(Track(mediaTrack, nullptr));
                }
              }
              {
                const auto recinput = (int) reaper::GetMediaTrackInfo_Value(mediaTrack, "I_RECINPUT");
                if (td->recinput != recinput) {
                  td->recinput = recinput;
                  trackInputChangedSubject_.get_subscriber().on_next(Track(mediaTrack, nullptr));
                }
              }
            }
          }
          return 0;
        }
        // DONE-rust
        case CSURF_EXT_SETFXPARAM: {
          if (!parm1 || !parm2 || !parm3) {
            return 0;
          }
          fxParamSet(parm1, parm2, parm3, false);
          return 0;
        }
        // DONE-rust
        case CSURF_EXT_SETFXPARAM_RECFX: {
          if (!parm1 || !parm2 || !parm3) {
            return 0;
          }
          fxParamSet(parm1, parm2, parm3, true);
          return 0;
        }
        // DONE-rust
        case CSURF_EXT_SETFXENABLED: {
          if (!parm1 || !parm2) {
            return 0;
          }
          const auto mediaTrack = (MediaTrack*) parm1;
          const auto parmFxIndex = *(int*) parm2;
          // Unfortunately, we don't have a ReaProject* here. Therefore we pass a nullptr.
          const Track track(mediaTrack, nullptr);
          if (const auto fx = getFxFromParmFxIndex(track, parmFxIndex)) {
            fxEnabledChangedSubject_.get_subscriber().on_next(*fx);
          }
          return 0;
        }
        // DONE-rust
        case CSURF_EXT_SETSENDVOLUME:
        case CSURF_EXT_SETSENDPAN: {
          const auto mediaTrack = (MediaTrack*) parm1;
          const int sendIdx = *(int*) parm2;
          const Track track(mediaTrack, nullptr);
          const auto trackSend = track.indexBasedSendByIndex(sendIdx);
          if (call == CSURF_EXT_SETSENDVOLUME) {
            trackSendVolumeChangedSubject_.get_subscriber().on_next(trackSend);
            // Send volume touch event only if not automated
            if (!trackParameterIsAutomated(track, "Send Volume")) {
              trackSendVolumeTouchedSubject_.get_subscriber().on_next(trackSend);
            }
          } else if (call == CSURF_EXT_SETSENDPAN) {
            trackSendPanChangedSubject_.get_subscriber().on_next(trackSend);
            // Send pan touch event only if not automated
            if (!trackParameterIsAutomated(track, "Send Pan")) {
              trackSendPanTouchedSubject_.get_subscriber().on_next(trackSend);
            }
          }
          return 0;
        }
          // DONE-rust
        case CSURF_EXT_SETFOCUSEDFX: {
          if (!parm1 || parm2 || !parm3) {
            // Clear focused FX
            fxFocusedSubject_.get_subscriber().on_next(none);
            return 0;
          }
          const auto mediaTrack = (MediaTrack*) parm1;
          const int parmFxIndex = *(int*) parm3;
          // Unfortunately, we don't have a ReaProject* here. Therefore we pass a nullptr.
          const Track track(mediaTrack, nullptr);
          if (const auto fx = getFxFromParmFxIndex(track, parmFxIndex)) {
            // Because CSURF_EXT_SETFXCHANGE doesn't fire if FX pasted in REAPER < 5.95-pre2 and on chunk manipulations
            detectFxChangesOnTrack(Track(mediaTrack, nullptr), true, !fx->isInputFx(), fx->isInputFx());
            fxFocusedSubject_.get_subscriber().on_next(*fx);
          }
          return 0;
        }
        // DONE-rust
        case CSURF_EXT_SETFXOPEN: {
          if (!parm1 || !parm2) {
            return 0;
          }
          const auto mediaTrack = (MediaTrack*) parm1;
          const int parmFxIndex = *(int*) parm2;
          // Unfortunately, we don't have a ReaProject* here. Therefore we pass a nullptr.
          const Track track(mediaTrack, nullptr);
          if (const auto fx = getFxFromParmFxIndex(track, parmFxIndex)) {
            // Because CSURF_EXT_SETFXCHANGE doesn't fire if FX pasted in REAPER < 5.95-pre2 and on chunk manipulations
            detectFxChangesOnTrack(Track(mediaTrack, nullptr), true, !fx->isInputFx(), fx->isInputFx());
            if (parm3 == 0) {
              fxClosedSubject_.get_subscriber().on_next(*fx);
            } else {
              fxOpenedSubject_.get_subscriber().on_next(*fx);
            }
          }
          return 0;
        }
          // DONE-rust
        case CSURF_EXT_SETFXCHANGE: {
          if (!parm1) {
            return 0;
          }
          const auto mediaTrack = (MediaTrack*) parm1;
          if (supportsDetectionOfInputFxInSetFxChange_) {
            const auto flags = (intptr_t) parm2;
            const bool isInputFx = (flags & 1) == 1;
            detectFxChangesOnTrack(Track(mediaTrack, nullptr), true, !isInputFx, isInputFx);
          } else {
            // REAPER < 5.95, we don't know if the change happened on input or normal FX chain
            detectFxChangesOnTrack(Track(mediaTrack, nullptr), true, true, true);
          }
          return 0;
        }
          // DONE-rust
        case CSURF_EXT_SETLASTTOUCHEDFX: {
          fxHasBeenTouchedJustAMomentAgo_ = true;
          return 0;
        }
          // DONE-rust
        case CSURF_EXT_SETBPMANDPLAYRATE: {
          if (parm1) {
            masterTempoChangedSubject_.get_subscriber().on_next(true);
            // If there's a tempo envelope, there are just tempo notifications when the tempo is actually changed.
            // So that's okay for "touched".
            // TODO What about gradual tempo changes?
            masterTempoTouchedSubject_.get_subscriber().on_next(true);
          }
          if (parm2) {
            masterPlayrateChangedSubject_.get_subscriber().on_next(true);
            // FIXME What about playrate automation?
            masterPlayrateTouchedSubject_.get_subscriber().on_next(true);
          }
          return 0;
        }
        default:return 0;
      }
    } catch (...) {
      logException();
    }
  }
  void HelperControlSurface::fxParamSet(void* parm1, void* parm2, void* parm3, bool isInputFxIfSupported) {
    const auto mediaTrack = (MediaTrack*) parm1;
    const auto fxAndParamIndex = *static_cast<int*>(parm2);
    const int fxIndex = (fxAndParamIndex >> 16) & 0xffff;
    const int paramIndex = fxAndParamIndex & 0xffff;
    // Unfortunately, we don't have a ReaProject* here. Therefore we pass a nullptr.
    const Track track(mediaTrack, nullptr);
    const double paramValue = *(double*) parm3;
    const bool isInputFx = supportsDetectionOfInputFx_
                           ? isInputFxIfSupported
                           : isProbablyInputFx(track, fxIndex, paramIndex, paramValue);
    const auto fxChain = isInputFx ? track.inputFxChain() : track.normalFxChain();
    if (const auto fx = fxChain.fxByIndex(fxIndex)) {
      const auto fxParam = fx->parameterByIndex(paramIndex);
      fxParameterValueChangedSubject_.get_subscriber().on_next(fxParam);
      if (fxHasBeenTouchedJustAMomentAgo_) {
        fxHasBeenTouchedJustAMomentAgo_ = false;
        fxParameterTouchedSubject_.get_subscriber().on_next(fxParam);
      }
    }
  }

  rxcpp::observable<bool> HelperControlSurface::masterTempoChanged() const {
    return masterTempoChangedSubject_.get_observable();
  }

  rxcpp::observable<bool> HelperControlSurface::masterTempoTouched() const {
    return masterTempoTouchedSubject_.get_observable();
  }

  rxcpp::observable<bool> HelperControlSurface::masterPlayrateChanged() const {
    return masterPlayrateChangedSubject_.get_observable();
  }

  rxcpp::observable<bool> HelperControlSurface::masterPlayrateTouched() const {
    return masterPlayrateTouchedSubject_.get_observable();
  }

  rxcpp::observable<Track> HelperControlSurface::trackInputMonitoringChanged() const {
    return trackInputMonitoringChangedSubject_.get_observable();
  }

  rxcpp::observable<Track> HelperControlSurface::trackArmChanged() const {
    return trackArmChangedSubject_.get_observable();
  }

  rxcpp::observable<Track> HelperControlSurface::trackMuteChanged() const {
    return trackMuteChangedSubject_.get_observable();
  }

  rxcpp::observable<Track> HelperControlSurface::trackMuteTouched() const {
    return trackMuteTouchedSubject_.get_observable();
  }

  rxcpp::observable<Track> HelperControlSurface::trackSoloChanged() const {
    return trackSoloChangedSubject_.get_observable();
  }

  rxcpp::observable<Track> HelperControlSurface::trackSoloTouched() const {
    // So far there is no automation envelope for track solo, so touched = changed
    return trackSoloChanged();
  }

  rxcpp::observable<Track> HelperControlSurface::trackSelectedChanged() const {
    return trackSelectedChangedSubject_.get_observable();
  }

  rxcpp::observable<Track> HelperControlSurface::trackSelectedTouched() const {
    // So far there is no automation envelope for track selection, so touched = changed
    return trackSelectedChanged();
  }

  rxcpp::observable<Project> HelperControlSurface::projectSwitched() const {
    // A behavior always sends the current value on subscribe. We are not interested in that, not at all!
    return activeProjectBehavior_.get_observable().skip(1);
  }

  rxcpp::observable<Project> HelperControlSurface::projectClosed() const {
    return projectClosedSubject_.get_observable();
  }

  void HelperControlSurface::SetTrackListChange() {
    try {
      // FIXME Not multi-project compatible!
      const auto newActiveProject = Reaper::instance().currentProject();
      if (newActiveProject != activeProjectBehavior_.get_value()) {
        activeProjectBehavior_.get_subscriber().on_next(newActiveProject);
      }
      numTrackSetChangesLeftToBePropagated_ = reaper::CountTracks(nullptr) + 1;
      removeInvalidReaProjects();
      detectTrackSetChanges();
    } catch (...) {
      logException();
    }
  }

  HelperControlSurface::State HelperControlSurface::state() const {
    return numTrackSetChangesLeftToBePropagated_ == 0 ? State::Normal : State::PropagatingTrackSetChanges;
  }

  void HelperControlSurface::detectTrackSetChanges() {
    const auto project = Reaper::instance().currentProject();
    auto& oldTrackDatas = trackDataByMediaTrackByReaProject_[project.reaProject()];
    const auto oldTrackCount = (int) oldTrackDatas.size();
    const int newTrackCount = project.trackCount();
    if (newTrackCount < oldTrackCount) {
      removeInvalidMediaTracks(project, oldTrackDatas);
    } else if (newTrackCount > oldTrackCount) {
      addMissingMediaTracks(project, oldTrackDatas);
    } else {
      updateMediaTrackPositions(project, oldTrackDatas);
    }
  }

  void HelperControlSurface::addMissingMediaTracks(const Project& project, TrackDataMap& trackDatas) {
    project.tracks().subscribe(
        [this, &trackDatas](Track track) {
          auto mediaTrack = track.mediaTrack();
          if (trackDatas.count(mediaTrack) == 0) {
            TrackData d;
            d.recarm = reaper::GetMediaTrackInfo_Value(mediaTrack, "I_RECARM") != 0;
            d.mute = reaper::GetMediaTrackInfo_Value(mediaTrack, "B_MUTE") != 0;
            d.number = (int) (size_t) reaper::GetSetMediaTrackInfo(mediaTrack, "IP_TRACKNUMBER", nullptr);
            d.pan = reaper::GetMediaTrackInfo_Value(mediaTrack, "D_PAN");
            d.volume = reaper::GetMediaTrackInfo_Value(mediaTrack, "D_VOL");
            d.selected = reaper::GetMediaTrackInfo_Value(mediaTrack, "I_SELECTED") != 0;
            d.solo = reaper::GetMediaTrackInfo_Value(mediaTrack, "I_SOLO") != 0;
            d.recmonitor = (int) reaper::GetMediaTrackInfo_Value(mediaTrack, "I_RECMON");
            d.recinput = (int) reaper::GetMediaTrackInfo_Value(mediaTrack, "I_RECINPUT");
            d.guid = Track::getMediaTrackGuid(mediaTrack);
            trackDatas[mediaTrack] = d;
            trackAddedSubject_.get_subscriber().on_next(track);
            detectFxChangesOnTrack(track, false, true, true);
          }
        },
        util::getLoggingErrorHandler()
    );
  }

  void HelperControlSurface::updateMediaTrackPositions(const Project& project,
      HelperControlSurface::TrackDataMap& trackDatas) {
    bool tracksHaveBeenReordered = false;
    for (auto it = trackDatas.begin(); it != trackDatas.end();) {
      const auto mediaTrack = it->first;
      if (reaper::ValidatePtr2(project.reaProject(), (void*) mediaTrack, "MediaTrack*")) {
        auto& trackData = it->second;
        const auto newNumber = (int) (size_t) reaper::GetSetMediaTrackInfo(mediaTrack, "IP_TRACKNUMBER", nullptr);
        if (newNumber != trackData.number) {
          tracksHaveBeenReordered = true;
          trackData.number = newNumber;
        }
      }
      it++;
    }
    if (tracksHaveBeenReordered) {
      tracksReorderedSubject_.get_subscriber().on_next(project);
    }
  }

  TrackData* HelperControlSurface::findTrackDataByTrack(MediaTrack* mediaTrack) {
    const auto project = Reaper::instance().currentProject();
    auto& trackDatas = trackDataByMediaTrackByReaProject_[project.reaProject()];
    if (trackDatas.count(mediaTrack) == 0) {
      return nullptr;
    } else {
      return &trackDatas.at(mediaTrack);
    }
  }

  void HelperControlSurface::SetSurfaceMute(MediaTrack* trackid, bool mute) {
    try {
      if (state() != State::PropagatingTrackSetChanges) {
        if (auto td = findTrackDataByTrack(trackid)) {
          if (td->mute != mute) {
            td->mute = mute;
            Track track(trackid, nullptr);
            trackMuteChangedSubject_.get_subscriber().on_next(track);
            if (!trackParameterIsAutomated(track, "Mute")) {
              trackMuteTouchedSubject_.get_subscriber().on_next(track);
            }
          }
        }
      }
    } catch (...) {
      logException();
    }
  }

  void HelperControlSurface::SetSurfaceSelected(MediaTrack* trackid, bool selected) {
    try {
      if (state() != State::PropagatingTrackSetChanges) {
        if (auto td = findTrackDataByTrack(trackid)) {
          if (td->selected != selected) {
            td->selected = selected;
            Track track(trackid, nullptr);
            trackSelectedChangedSubject_.get_subscriber().on_next(track);
          }
        }
      }
    } catch (...) {
      logException();
    }
  }

  void HelperControlSurface::SetSurfaceSolo(MediaTrack* trackid, bool solo) {
    try {
      if (state() != State::PropagatingTrackSetChanges) {
        if (auto td = findTrackDataByTrack(trackid)) {
          if (td->solo != solo) {
            td->solo = solo;
            Track track(trackid, nullptr);
            trackSoloChangedSubject_.get_subscriber().on_next(track);
          }
        }
      }
    } catch (...) {
      logException();
    }
  }

  void HelperControlSurface::SetSurfaceRecArm(MediaTrack* trackid, bool recarm) {
    try {
      if (state() != State::PropagatingTrackSetChanges) {
        if (auto td = findTrackDataByTrack(trackid)) {
          if (td->recarm != recarm) {
            td->recarm = recarm;
            Track track(trackid, nullptr);
            trackArmChangedSubject_.get_subscriber().on_next(track);
          }
        }
      }
    } catch (...) {
      logException();
    }
  }

  void HelperControlSurface::removeInvalidMediaTracks(const Project& project, TrackDataMap& trackDatas) {
    for (auto it = trackDatas.begin(); it != trackDatas.end();) {
      const auto mediaTrack = it->first;
      const auto trackData = it->second;
      if (reaper::ValidatePtr2(project.reaProject(), (void*) mediaTrack, "MediaTrack*")) {
        it++;
      } else {
        fxChainPairByMediaTrack_.erase(mediaTrack);
        trackRemovedSubject_.get_subscriber().on_next(project.trackByGuid(trackData.guid));
        it = trackDatas.erase(it);
      }
    }
  }

  void HelperControlSurface::removeInvalidReaProjects() {
    for (auto it = trackDataByMediaTrackByReaProject_.begin(); it != trackDataByMediaTrackByReaProject_.end();) {
      const auto pair = *it;
      const auto project = pair.first;
      if (reaper::ValidatePtr2(nullptr, (void*) project, "ReaProject*")) {
        it++;
      } else {
        projectClosedSubject_.get_subscriber().on_next(Project(project));
        it = trackDataByMediaTrackByReaProject_.erase(it);
      }
    }
  }

  rx::observable<Track> HelperControlSurface::trackRemoved() const {
    return trackRemovedSubject_.get_observable();
  }

  rx::observable<Track> HelperControlSurface::trackAdded() const {
    return trackAddedSubject_.get_observable();
  }

  rx::observable<Track> HelperControlSurface::trackVolumeChanged() const {
    return trackVolumeChangedSubject_.get_observable();
  }

  rx::observable<Track> HelperControlSurface::trackPanChanged() const {
    return trackPanChangedSubject_.get_observable();
  }

  rxcpp::observable<Track> HelperControlSurface::trackNameChanged() const {
    return trackNameChangedSubject_.get_observable();
  }

  rxcpp::observable<Track> HelperControlSurface::trackInputChanged() const {
    return trackInputChangedSubject_.get_observable();
  }

  rx::observable<TrackSend> HelperControlSurface::trackSendVolumeChanged() const {
    return trackSendVolumeChangedSubject_.get_observable();
  }

  rxcpp::observable<TrackSend> HelperControlSurface::trackSendPanChanged() const {
    return trackSendPanChangedSubject_.get_observable();
  }

  rxcpp::observable<TrackSend> HelperControlSurface::trackSendPanTouched() const {
    return trackSendPanTouchedSubject_.get_observable();
  }

  const rxcpp::observe_on_one_worker& HelperControlSurface::mainThreadCoordination() const {
    return mainThreadCoordination_;
  }

  void HelperControlSurface::init() {
    HelperControlSurface::instance();
  }

  rx::observable<Track> HelperControlSurface::fxReordered() const {
    return fxReorderedSubject_.get_observable();
  }
  rxcpp::observable<Fx> HelperControlSurface::fxOpened() const {
    return fxOpenedSubject_.get_observable();
  }
  rxcpp::observable<Fx> HelperControlSurface::fxClosed() const {
    return fxClosedSubject_.get_observable();
  }
  rxcpp::observable<boost::optional<Fx>> HelperControlSurface::fxFocused() const {
    return fxFocusedSubject_.get_observable();
  }

  void HelperControlSurface::detectFxChangesOnTrack(Track track, bool notifyListenersAboutChanges,
      bool checkNormalFxChain, bool checkInputFxChain) {
    if (track.isAvailable()) {
      MediaTrack* mediaTrack = track.mediaTrack();
      auto& fxChainPair = fxChainPairByMediaTrack_[mediaTrack];
      const bool addedOrRemovedOutputFx =
          checkNormalFxChain
          ? detectFxChangesOnTrack(track, fxChainPair.outputFxGuids, false, notifyListenersAboutChanges)
          : false;
      const bool addedOrRemovedInputFx =
          checkInputFxChain
          ? detectFxChangesOnTrack(track, fxChainPair.inputFxGuids, true, notifyListenersAboutChanges)
          : false;
      if (notifyListenersAboutChanges && !addedOrRemovedInputFx && !addedOrRemovedOutputFx) {
        fxReorderedSubject_.get_subscriber().on_next(track);
      }
    }
  }

  bool HelperControlSurface::detectFxChangesOnTrack(Track track,
      set<string>& oldFxGuids,
      bool isInputFx,
      bool notifyListenersAboutChanges) {
    const auto oldFxCount = (int) oldFxGuids.size();
    const int newFxCount = (isInputFx ? track.inputFxChain() : track.normalFxChain()).fxCount();
    if (newFxCount < oldFxCount) {
      removeInvalidFx(track, oldFxGuids, isInputFx, notifyListenersAboutChanges);
      return true;
    } else if (newFxCount > oldFxCount) {
      addMissingFx(track, oldFxGuids, isInputFx, notifyListenersAboutChanges);
      return true;
    } else {
      // Reordering (or nothing)
      return false;
    }
  }

  void HelperControlSurface::removeInvalidFx(Track track,
      std::set<string>& oldFxGuids,
      bool isInputFx,
      bool notifyListenersAboutChanges) {
    const auto newFxGuids = fxGuidsOnTrack(track, isInputFx);
    for (auto it = oldFxGuids.begin(); it != oldFxGuids.end();) {
      const string oldFxGuid = *it;
      if (newFxGuids.count(oldFxGuid)) {
        it++;
      } else {
        if (notifyListenersAboutChanges) {
          const auto fxChain = isInputFx ? track.inputFxChain() : track.normalFxChain();
          fxRemovedSubject_.get_subscriber().on_next(fxChain.fxByGuid(oldFxGuid));
        }
        it = oldFxGuids.erase(it);
      }
    }
  }

  void HelperControlSurface::addMissingFx(Track track,
      std::set<string>& fxGuids,
      bool isInputFx,
      bool notifyListenersAboutChanges) {
    const auto fxChain = isInputFx ? track.inputFxChain() : track.normalFxChain();
    fxChain.fxs().subscribe(
        [this, &fxGuids, notifyListenersAboutChanges](Fx fx) {
          bool wasInserted = fxGuids.insert(fx.guid()).second;
          if (wasInserted && notifyListenersAboutChanges) {
            fxAddedSubject_.get_subscriber().on_next(fx);
          }
        },
        util::getLoggingErrorHandler()
    );
  }

  std::set<string> HelperControlSurface::fxGuidsOnTrack(Track track, bool isInputFx) const {
    const auto fxChain = isInputFx ? track.inputFxChain() : track.normalFxChain();
    std::set<string> fxGuids;
    fxChain.fxs().subscribe(
        [&fxGuids](Fx fx) {
          fxGuids.insert(fx.guid());
        },
        util::getLoggingErrorHandler()
    );
    return fxGuids;
  }

  rx::observable<Fx> HelperControlSurface::fxAdded() const {
    return fxAddedSubject_.get_observable();
  }

  rx::observable<Fx> HelperControlSurface::fxRemoved() const {
    return fxRemovedSubject_.get_observable();
  }

  rx::observable<Project> HelperControlSurface::tracksReordered() const {
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

  rx::observable<Parameter*> HelperControlSurface::parameterValueChangedUnsafe() const {
    return fxParameterValueChanged().map([this](FxParameter fxParam) -> Parameter* {
          return new FxParameter(fxParam);
        })
        .merge(fxEnabledChanged().map([this](Fx fx) -> Parameter* {
          return new FxEnable(fx);
        }))
        .merge(trackVolumeChanged().map([this](Track track) -> Parameter* {
          return new TrackVolume(track);
        }))
        .merge(trackPanChanged().map([this](Track track) -> Parameter* {
          return new TrackPan(track);
        }))
        .merge(trackArmChanged().map([this](Track track) -> Parameter* {
          return new TrackArm(track);
        }))
        .merge(trackMuteChanged().map([this](Track track) -> Parameter* {
          return new TrackMute(track);
        }))
        .merge(trackSoloChanged().map([this](Track track) -> Parameter* {
          return new TrackSolo(track);
        }))
        .merge(trackSelectedChanged().map([this](Track track) -> Parameter* {
          return new TrackSelection(track);
        }))
        .merge(masterTempoChanged().map([](bool) -> Parameter* {
          return new MasterTempo();
        }))
        .merge(masterPlayrateChanged().map([](bool) -> Parameter* {
          return new MasterPlayrate();
        }))
        .merge(trackSendPanChanged().map([this](TrackSend trackSend) -> Parameter* {
          return new TrackSendPan(trackSend);
        }))
        .merge(trackSendVolumeChanged().map([this](TrackSend trackSend) -> Parameter* {
          return new TrackSendVolume(trackSend);
        }));
  }

  rx::observable<Parameter*> HelperControlSurface::parameterTouchedUnsafe() const {
    return fxParameterTouched().map([this](FxParameter fxParam) -> Parameter* {
          return new FxParameter(fxParam);
        })
        .merge(fxEnabledTouched().map([this](Fx fx) -> Parameter* {
          return new FxEnable(fx);
        }))
        .merge(trackVolumeTouched().map([this](Track track) -> Parameter* {
          return new TrackVolume(track);
        }))
        .merge(trackPanTouched().map([this](Track track) -> Parameter* {
          return new TrackPan(track);
        }))
        .merge(trackArmTouched().map([this](Track track) -> Parameter* {
          return new TrackArm(track);
        }))
        .merge(trackMuteTouched().map([this](Track track) -> Parameter* {
          return new TrackMute(track);
        }))
        .merge(trackSoloTouched().map([this](Track track) -> Parameter* {
          return new TrackSolo(track);
        }))
        .merge(trackSelectedTouched().map([this](Track track) -> Parameter* {
          return new TrackSelection(track);
        }))
        .merge(masterTempoTouched().map([](bool) -> Parameter* {
          return new MasterTempo();
        }))
        .merge(masterPlayrateTouched().map([](bool) -> Parameter* {
          return new MasterPlayrate();
        }))
        .merge(trackSendPanTouched().map([this](TrackSend trackSend) -> Parameter* {
          return new TrackSendPan(trackSend);
        }))
        .merge(trackSendVolumeTouched().map([this](TrackSend trackSend) -> Parameter* {
          return new TrackSendVolume(trackSend);
        }));
  }

  rx::observable<FxParameter> HelperControlSurface::fxParameterTouched() const {
    return fxParameterTouchedSubject_.get_observable();
  }

  rx::observable<Track> HelperControlSurface::trackVolumeTouched() const {
    return trackVolumeTouchedSubject_.get_observable();
  }

  rx::observable<Track> HelperControlSurface::trackPanTouched() const {
    return trackPanTouchedSubject_.get_observable();
  }

  rx::observable<Track> HelperControlSurface::trackArmTouched() const {
    // So far there is no automation envelope for track arm, so touched = changed
    return trackArmChanged();
  }

  rx::observable<TrackSend> HelperControlSurface::trackSendVolumeTouched() const {
    return trackSendVolumeTouchedSubject_.get_observable();
  }
  rxcpp::observable<bool> HelperControlSurface::mainThreadIdle() const {
    return mainThreadIdleSubject_.get_observable();
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