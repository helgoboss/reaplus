#include <memory>
#include "Track.h"
#include "Fx.h"
#include "TrackSend.h"
#include "HelperControlSurface.h"
#include "utility.h"
#include "Project.h"
#include "Reaper.h"
#include "FxChain.h"

#include "reaper_plugin_functions.h"

using std::shared_ptr;
using rxcpp::subscriber;
using rxcpp::observable;
using boost::none;
using boost::optional;
using std::string;
using std::function;
using std::unique_ptr;


namespace reaplus {

  const int Track::MAX_CHUNK_SIZE = 1000000;

  Track::Track(MediaTrack* mediaTrack, ReaProject* reaProject) :
      mediaTrack_(mediaTrack),
      reaProject_(reaProject),
      // We load the GUID eagerly because we want to make comparability possible even in the following case:
      // Track A has been initialized with a GUID not been loaded yet, track B has been initialized with a MediaTrack*
      // (this constructor) but has rendered invalid in the meantime. Now there would not be any way to compare them
      // because I can neither compare MediaTrack* pointers nor GUIDs. Except I extract the GUID eagerly.
      guid_(Track::getMediaTrackGuid(mediaTrack)) {
  }

  MediaTrack* Track::mediaTrack() const {
    loadIfNecessaryOrComplain();
    return mediaTrack_;
  }

  std::string Track::getMediaTrackGuid(MediaTrack* mediaTrack) {
    auto guid = (GUID*) reaper::GetSetMediaTrackInfo(mediaTrack, "GUID", nullptr);
    return convertGuidToString(*guid);
  }

  string Track::guid() const {
    return guid_;
  }

  int Track::index() const {
    loadAndCheckIfNecessaryOrComplain();
    auto ipTrackNumber = (int)(size_t) reaper::GetSetMediaTrackInfo(mediaTrack(), "IP_TRACKNUMBER", nullptr);
    if (ipTrackNumber == -1) {
      // Master track indicator
      return -1;
    } else {
      // Must be > 0 (is 0 if track not found which is impossible because we checked at the beginning)
      return ipTrackNumber - 1;
    }
  }

  string Track::name() const {
    loadAndCheckIfNecessaryOrComplain();
    if (isMasterTrack()) {
      return "<Master track>";
    } else {
      const auto name = (char*) reaper::GetSetMediaTrackInfo(mediaTrack(), "P_NAME", nullptr);
      return string(name);
    }
  }

  void Track::setVolume(double normalizedValue) {
    loadAndCheckIfNecessaryOrComplain();
    const double reaperValue = Volume(normalizedValue).reaperValue();
    // CSurf_OnVolumeChangeEx has a slightly lower precision than setting D_VOL directly. The return value
    // reflects the cropped value. The precision became much better with REAPER 5.28.
    reaper::CSurf_OnVolumeChangeEx(mediaTrack(), reaperValue, false, false);
  }

  Volume Track::volume() const {
    loadAndCheckIfNecessaryOrComplain();
    // It's important that we don't query D_VOL because that returns the wrong value in case an envelope is written
    double volume;
    reaper::GetTrackUIVolPan(mediaTrack(), &volume, nullptr);
    return Volume::ofReaperValue(volume);
  }

  void Track::setPan(double normalizedValue) {
    loadAndCheckIfNecessaryOrComplain();
    const double reaperValue = Pan(normalizedValue).reaperValue();
    reaper::CSurf_OnPanChangeEx(mediaTrack(), reaperValue, false, false);
  }

  Pan Track::pan() const {
    loadAndCheckIfNecessaryOrComplain();
    // It's important that we don't query D_PAN because that returns the wrong value in case an envelope is written
    double pan;
    reaper::GetTrackUIVolPan(mediaTrack(), nullptr, &pan);
    return Pan::ofReaperValue(pan);
  }

  bool Track::isMasterTrack() const {
    return index() == -1;
  }

  ReaProject* Track::findContainingProject() const {
    if (mediaTrack_ == nullptr) {
      throw std::logic_error("Containing project cannot be found if mediaTrack not available");
    }
    // No ReaProject* available. Try current project first (most likely in everyday REAPER usage).
    const auto currentProject = Reaper::instance().currentProject();
    const bool isValidInCurrentProject = reaper::ValidatePtr2(currentProject.reaProject(), mediaTrack_,
        "MediaTrack*");
    if (isValidInCurrentProject) {
      // Valid in current project
      return currentProject.reaProject();
    } else {
      // Worst case. It could still be valid in another project. We have to check each project.
      const auto otherProject = Reaper::instance().projects()
          .filter([currentProject](Project p) {
            // We already know it's invalid in current project
            return p != currentProject;
          })
          .filter([this](Project p) {
            return reaper::ValidatePtr2(p.reaProject(), mediaTrack_, "MediaTrack*");
          })
          .map([](Project p) {
            return shared_ptr<Project>(new Project(p));
          })
          .default_if_empty((shared_ptr<Project>)nullptr)
          .as_blocking()
          .first();
      if (otherProject == nullptr) {
        return nullptr;
      } else {
        // Speed up validation process for next check
        return otherProject->reaProject();
      }
    }
  }

  void Track::attemptToFillProjectIfNecessary() const {
    if (reaProject_ == nullptr) {
      reaProject_ = findContainingProject();
    }
  }

  bool Track::isValid() const {
    if (mediaTrack_ == nullptr) {
      throw std::logic_error("Track can not be validated if mediaTrack not available");
    }
    attemptToFillProjectIfNecessary();
    if (reaProject_ == nullptr) {
      return false;
    } else {
      if (Project(reaProject_).isAvailable()) {
        return reaper::ValidatePtr2(reaProject_, mediaTrack_, "MediaTrack*");
      } else {
        return false;
      }
    }
  }

  bool operator==(const Track& lhs, const Track& rhs) {
    if (lhs.mediaTrack_ && rhs.mediaTrack_) {
      return lhs.mediaTrack_ == rhs.mediaTrack_;
    } else {
      return lhs.guid() == rhs.guid();
    }
  }

  Project Track::project() const {
    loadIfNecessaryOrComplain();
    return uncheckedProject();
  }

  Project Track::uncheckedProject() const {
    attemptToFillProjectIfNecessary();
    return Project(reaProject_);
  }

  boost::optional<ChunkRegion> Track::autoArmChunkLine() const {
    return Track::autoArmChunkLine(chunk());
  }

  bool Track::hasAutoArmEnabled() const {
    loadAndCheckIfNecessaryOrComplain();
    return autoArmChunkLine().is_initialized();
  }

  void Track::enableAutoArm() {
    auto chunk = this->chunk();
    auto autoArmChunkLine = Track::autoArmChunkLine(chunk);
    if (!autoArmChunkLine) {
      const bool wasArmedBefore = isArmed();
      chunk.insertAfterRegionAsBlock(chunk.region().firstLine(), "AUTO_RECARM 1");
      setChunk(chunk);
      if (wasArmedBefore) {
        arm();
      } else {
        disarm();
      }
    }
  }

  void Track::select() {
    loadAndCheckIfNecessaryOrComplain();
    reaper::SetTrackSelected(mediaTrack(), true);
  }

  void Track::selectExclusively() {
    loadAndCheckIfNecessaryOrComplain();
    reaper::SetOnlyTrackSelected(mediaTrack());
  }

  bool Track::isSelected() const {
    loadAndCheckIfNecessaryOrComplain();
    return reaper::GetMediaTrackInfo_Value(mediaTrack(), "I_SELECTED") == 1;
  }

  void Track::unselect() {
    loadAndCheckIfNecessaryOrComplain();
    reaper::SetTrackSelected(mediaTrack(), false);
  }

  void Track::disableAutoArm() {
    auto autoArmChunkLine = this->autoArmChunkLine();
    if (autoArmChunkLine) {
      auto chunk = autoArmChunkLine->parentChunk();
      chunk.deleteRegion(*autoArmChunkLine);
      setChunk(chunk);
    }
  }

  void Track::setChunk(Chunk chunk) {
    setChunk(chunk.content()->c_str());
  }

  void Track::arm(bool supportAutoArm) {
    if (supportAutoArm && hasAutoArmEnabled()) {
      select();
    } else {
      reaper::CSurf_OnRecArmChangeEx(mediaTrack(), 1, false);
    }
  }

  bool Track::isArmed(bool supportAutoArm) const {
    if (supportAutoArm && hasAutoArmEnabled()) {
      return isSelected();
    } else {
      loadAndCheckIfNecessaryOrComplain();
      return reaper::GetMediaTrackInfo_Value(mediaTrack(), "I_RECARM") == 1;
    }
  }

  void Track::setRecordingInput(MidiRecordingInput midiRecordingInput) {
    loadAndCheckIfNecessaryOrComplain();
    reaper::SetMediaTrackInfo_Value(mediaTrack_, "I_RECINPUT", midiRecordingInput.recInputIndex());
    // Only for triggering notification (as manual setting the rec input would also trigger it)
    int recMon = (int) reaper::GetMediaTrackInfo_Value(mediaTrack_, "I_RECMON");
    HelperControlSurface::instance().Extended(CSURF_EXT_SETINPUTMONITOR, (void*) mediaTrack_, (void*) &recMon, nullptr);
    // This doesn't work in one test case unfortunately
//    reaper::CSurf_OnInputMonitorChangeEx(mediaTrack_, recMon, false);
  }

  AutomationMode Track::automationMode() const {
    loadAndCheckIfNecessaryOrComplain();
    return static_cast<AutomationMode>(reaper::GetTrackAutomationMode(mediaTrack()));
  }

  AutomationMode Track::effectiveAutomationMode() const {
    const auto automationOverride = Reaper::instance().globalAutomationOverride();
    if (automationOverride == AutomationMode::NoOverride) {
      return automationMode();
    } else {
      return automationOverride;
    }
  }

  FxChain Track::normalFxChain() const {
    return FxChain(*this, false);
  }

  FxChain Track::inputFxChain() const {
    return FxChain(*this, true);
  }

  int Track::sendCount() const {
    loadAndCheckIfNecessaryOrComplain();
    return reaper::GetTrackNumSends(mediaTrack(), 0);
  }

  observable<TrackSend> Track::sends() const {
    loadAndCheckIfNecessaryOrComplain();
    return observable<>::create<TrackSend>([this](subscriber<TrackSend> s) {
      const int sendCount = this->sendCount();
      for (int i = 0; i < sendCount && s.is_subscribed(); i++) {
        // Create a stable send (based on target track)
        const auto send = TrackSend(*this, TrackSend::targetTrack(*this, i), i);
        s.on_next(send);
      }
      s.on_completed();
    });
  }

  bool Track::isAvailable() const {
    if (mediaTrack_) {
      // Loaded
      return isValid();
    } else {
      // Not yet loaded
      return loadByGuid();
    }
  }

  Track::Track(Project project, string guid) : reaProject_(project.reaProject()), guid_(guid), mediaTrack_(nullptr) {

  }

  bool Track::loadByGuid() const {
    if (reaProject_ == nullptr) {
      throw std::logic_error("For loading per GUID, a project must be given");
    }
    // TODO Don't save ReaProject but Project as member
    mediaTrack_ = uncheckedProject().tracks()
        .filter([this](Track track) {
          return track.guid() == guid();
        })
        .map([](Track track) {
          return track.mediaTrack();
        })
        .default_if_empty((MediaTrack*) nullptr)
        .as_blocking()
        .first();
    return mediaTrack_ != nullptr;
  }

  bool reaplus::operator!=(const Track& lhs, const Track& rhs) {
    return !(lhs == rhs);
  }

  TrackSend Track::indexBasedSendByIndex(int index) const {
    return TrackSend(*this, index);
  }

  optional<TrackSend> Track::sendByIndex(int index) const {
    if (index < sendCount()) {
      return TrackSend(*this, TrackSend::targetTrack(*this, index), index);
    } else {
      return boost::none;
    }
  }

  bool Track::isMuted() const {
    loadAndCheckIfNecessaryOrComplain();
    return reaper::GetMediaTrackInfo_Value(mediaTrack(), "B_MUTE") == 1;
  }

  void Track::mute() {
    loadAndCheckIfNecessaryOrComplain();
    // TODO Add second version with surface mute
    reaper::SetMediaTrackInfo_Value(mediaTrack(), "B_MUTE", 1);
  }

  void Track::unmute() {
    loadAndCheckIfNecessaryOrComplain();
    // TODO Add second version with surface mute
    reaper::SetMediaTrackInfo_Value(mediaTrack(), "B_MUTE", 0);
  }

  void Track::loadAndCheckIfNecessaryOrComplain() const {
    loadIfNecessaryOrComplain();
    complainIfNotValid();
  }

  void Track::complainIfNotValid() const {
    if (!isValid()) {
      throw std::logic_error("Track not available");
    }
  }

  void Track::loadIfNecessaryOrComplain() const {
    if (mediaTrack_ == nullptr && !loadByGuid()) {
      throw std::logic_error("Track not loadable");
    }
  }

  optional<Fx> Track::fxByQueryIndex(int queryIndex) const {
    const auto pair = Fx::indexFromQueryIndex(queryIndex);
    const int index = pair.first;
    const int isInputFx = pair.second;
    const auto fxChain = isInputFx ? inputFxChain() : normalFxChain();
    return fxChain.fxByIndex(index);
  }

  void Track::setName(const string& name) {
    loadAndCheckIfNecessaryOrComplain();
    reaper::GetSetMediaTrackInfo(mediaTrack_, "P_NAME", &(const_cast<string&>(name))[0]);
  }

  std::unique_ptr<RecordingInput> Track::recordingInput() const {
    loadAndCheckIfNecessaryOrComplain();
    const int recInputIndex = *(int*) reaper::GetSetMediaTrackInfo(mediaTrack_, "I_RECINPUT", nullptr);
    return RecordingInput::ofRecInputIndex(recInputIndex);
  }

  void Track::disarm(bool supportAutoArm) {
    if (supportAutoArm && hasAutoArmEnabled()) {
      unselect();
    } else {
      reaper::CSurf_OnRecArmChangeEx(mediaTrack(), 0, false);
    }
  }

  TrackSend Track::sendByTargetTrack(Track targetTrack) const {
    return TrackSend(*this, targetTrack, boost::none);
  }

  TrackSend Track::addSendTo(Track targetTrack) {
    // FIXME Check how this behaves if send already exists
    int sendIndex = reaper::CreateTrackSend(mediaTrack(), targetTrack.mediaTrack());
    return TrackSend(*this, targetTrack, sendIndex);
  }

  Chunk Track::chunk(int maxChunkSize) const {
    const auto chunkString = reaplus::toSharedString(maxChunkSize, [this](char* buffer, int maxSize) {
      reaper::GetTrackStateChunk(mediaTrack(), buffer, maxSize, false);
    });
    return Chunk(chunkString);
  }

  void Track::setChunk(const char* chunk) {
    reaper::SetTrackStateChunk(mediaTrack(), chunk, true);
  }

  optional<ChunkRegion> Track::autoArmChunkLine(Chunk chunk) {
    return chunk.region().findLineStartingWith("AUTO_RECARM 1");
  }


}