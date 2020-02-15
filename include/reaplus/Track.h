#pragma once

#include <reaper_plugin.h>
#include <string>
#include <functional>
#include <memory>
#include <boost/optional.hpp>
#include <rxcpp/rx.hpp>
#include "Volume.h"
#include "Pan.h"
#include "Project.h"
#include "AutomationMode.h"
#include "RecordingInput.h"
#include "Chunk.h"

namespace reaplus {
  class Fx;
  class FxChain;
  class TrackSend;
  // DONE-rust
  enum class InputMonitoringMode {
    Off,
    Normal,
    NotWhenPlaying
  };
  // Tiny, not so expensive to copy wrapper around REAPER MediaTrack*.
  class Track {
    friend class Project;
  private:
    // DONE-rust
    // TODO Do we really need this pointer? Makes copying a tiny bit more expensive than just copying a MediaTrack*.
    mutable ReaProject* reaProject_;
    // Only filled if track loaded.
    mutable MediaTrack* mediaTrack_;
    // Possible states:
    // a) guid, project, !mediaTrack (guid-based and not yet loaded)
    // b) guid, mediaTrack (guid-based and loaded)
    // TODO This is not super cheap to copy. Do we really need to initialize this eagerly?
    std::string guid_;
  public:
    static int const MAX_CHUNK_SIZE;
    // DONE-rust
    static std::string getMediaTrackGuid(MediaTrack* mediaTrack);
    // DONE-rust
    // mediaTrack must not be null
    // reaProject can be null but providing it can speed things up quite much for REAPER versions < 5.95
    Track(MediaTrack* mediaTrack, ReaProject* reaProject);
    // DONE-rust
    MediaTrack* mediaTrack() const;
    // DONE-rust
    std::string guid() const;
    // DONE-rust
    Project project() const;
    // DONE-rust
    int index() const;
    // DONE-rust
    std::string name() const;
    // DONE-rust
    void setName(const std::string& name);
    // Returns nullptr if no recording input selected
    // DONE-rust
    std::unique_ptr<RecordingInput> recordingInput() const;
    // DONE-rust
    void setRecordingInput(MidiRecordingInput midiRecordingInput);
    // DONE-rust
    InputMonitoringMode inputMonitoringMode() const;
    // DONE-rust
    void setInputMonitoringMode(InputMonitoringMode inputMonitoringMode);
    // DONE-rust
    bool isMasterTrack() const;
    // DONE-rust
    void setVolume(double normalizedValue);
    // DONE-rust
    Volume volume() const;
    // DONE-rust
    void setPan(double normalizedValue);
    // DONE-rust
    Pan pan() const;
    // If supportAutoArm is false, auto-arm mode is disabled if it has been enabled before
    // DONE-rust
    void arm(bool supportAutoArm = true);
    // If supportAutoArm is false, auto-arm mode is disabled if it has been enabled before
    // DONE-rust
    void disarm(bool supportAutoArm = true);
    // DONE-rust
    bool hasAutoArmEnabled() const;
    // DONE-rust
    void enableAutoArm();
    // DONE-rust
    void disableAutoArm();
    // DONE-rust
    bool isArmed(bool supportAutoArm = true) const;
    // Attention! If you pass undoIsOptional = true it's faster but it returns a chunk that contains weird
    // FXID_NEXT (in front of FX tag) instead of FXID (behind FX tag). So FX chunk code should be double checked then.
    // DONE-rust
    Chunk chunk(int maxChunkSize = MAX_CHUNK_SIZE, bool undoIsOptional = false) const;
    // DONE-rust
    void setChunk(const char* chunk);
    // DONE-rust
    void setChunk(Chunk chunk);
    // DONE-rust
    void select();
    // Returns actual track to which has been scrolled (can be different)
    // TODO-rust
    boost::optional<Track> scrollMixer();
    // DONE-rust
    void unselect();
    // DONE-rust
    void selectExclusively();
    // DONE-rust
    bool isSelected() const;
    // DONE-rust
    FxChain normalFxChain() const;
    // DONE-rust
    FxChain inputFxChain() const;
    // It's correct that this returns an optional because the index isn't a stable identifier of an FX.
    // The FX could move. So this should do a runtime lookup of the FX and return a stable GUID-backed Fx object if
    // an FX exists at that query index.
    // DONE-rust
    boost::optional<Fx> fxByQueryIndex(int queryIndex) const;
    // DONE-rust
    bool isAvailable() const;
    // DONE-rust
    bool isMuted() const;
    // DONE-rust
    void mute();
    // DONE-rust
    void unmute();
    // DONE-rust
    bool isSolo() const;
    // DONE-rust
    void solo();
    // DONE-rust
    void unsolo();
    // Non-Optional. Even the index is not a stable identifier, we need a way to create
    // sends just by an index, not to target tracks. Think of ReaLearn for example and saving
    // a preset for a future project which doesn't have the same target track like in the
    // example project.
    // DONE-rust
    TrackSend indexBasedSendByIndex(int index) const;
    // DONE-rust
    TrackSend sendByTargetTrack(Track targetTrack) const;
    // DONE-rust
    TrackSend addSendTo(Track targetTrack);

    // DONE-rust
    int sendCount() const;
    // DONE-rust
    boost::optional<TrackSend> sendByIndex(int index) const;
    // Returns target-track based sends
    // DONE-rust
    rxcpp::observable<TrackSend> sends() const;
    // DONE-rust
    AutomationMode automationMode() const;
    // DONE-rust
    AutomationMode effectiveAutomationMode() const;
    // DONE-rust
    friend bool operator==(const Track& lhs, const Track& rhs);
    // DONE-rust
    friend bool operator!=(const Track& lhs, const Track& rhs);
  protected:
    // DONE-rust
    Track(Project project, std::string guid);
  private:
    // DONE-rust
    static boost::optional<ChunkRegion> autoArmChunkLine(Chunk chunk);

    // DONE-rust
    bool loadByGuid() const;

    // DONE-rust
    void loadAndCheckIfNecessaryOrComplain() const;

    // DONE-rust
    void loadIfNecessaryOrComplain() const;

    // DONE-rust
    // Precondition: mediaTrack_ must be filled!
    bool isValid() const;

    // DONE-rust
    boost::optional<ChunkRegion> autoArmChunkLine() const;

    // DONE-rust
    // Precondition: mediaTrack_ must be filled!
    ReaProject* findContainingProject() const;

    // DONE-rust
    // Precondition: mediaTrack_ must be filled!
    void attemptToFillProjectIfNecessary() const;

    // DONE-rust
    Project uncheckedProject() const;

    // DONE-rust
    void complainIfNotValid() const;
  };
}

