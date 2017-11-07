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
  class Track {
    friend class Project;
  private:
    mutable ReaProject* reaProject_;
    // Only filled if track loaded.
    mutable MediaTrack* mediaTrack_;
    // Possible states:
    // a) guid, project, !mediaTrack (guid-based and not yet loaded)
    // b) guid, mediaTrack (guid-based and loaded)
    // c) !guid, mediaTrack (media track based, loaded)
    boost::optional<std::string> guid_;
  public:
    static int const MAX_CHUNK_SIZE;
    // mediaTrack must not be null
    // reaProject can be null but should be set (for performance reasons)
    Track(MediaTrack* mediaTrack, ReaProject* reaProject);
    MediaTrack* mediaTrack() const;
    std::string guid() const;
    Project project() const;
    int index() const;
    std::string name() const;
    void setName(const std::string& name);
    // Returns nullptr if no recording input selected
    std::unique_ptr<RecordingInput> recordingInput() const;
    void setRecordingInput(MidiRecordingInput midiRecordingInput);
    bool isMasterTrack() const;
    void setVolume(double normalizedValue);
    Volume volume() const;
    void setPan(double normalizedValue);
    Pan pan() const;
    void arm(bool supportAutoArm = true);
    void disarm(bool supportAutoArm = true);
    bool hasAutoArmEnabled() const;
    void enableAutoArm();
    void disableAutoArm();
    bool isArmed(bool supportAutoArm = true) const;
    Chunk chunk(int maxChunkSize = MAX_CHUNK_SIZE) const;
    void setChunk(const char* chunk);
    void setChunk(Chunk chunk);
    void select();
    void unselect();
    void selectExclusively();
    bool isSelected() const;
    FxChain normalFxChain() const;
    FxChain inputFxChain() const;
    // It's correct that this returns an optional because the index isn't a stable identifier of an FX.
    // The FX could move. So this should do a runtime lookup of the FX and return a stable GUID-backed Fx object if
    // an FX exists at that query index.
    boost::optional<Fx> fxByQueryIndex(int queryIndex) const;
    bool isAvailable() const;
    bool isMuted() const;
    void mute();
    void unmute();
    // Non-Optional. Even the index is not a stable identifier, we need a way to create
    // sends just by an index, not to target tracks. Think of ReaLearn for example and saving
    // a preset for a future project which doesn't have the same target track like in the
    // example project.
    TrackSend indexBasedSendByIndex(int index) const;
    TrackSend sendByTargetTrack(Track targetTrack) const;
    TrackSend addSendTo(Track targetTrack);

    int sendCount() const;
    boost::optional<TrackSend> sendByIndex(int index) const;
    // Returns target-track based sends
    rxcpp::observable<TrackSend> sends() const;
    AutomationMode automationMode() const;
    AutomationMode effectiveAutomationMode() const;
    friend bool operator==(const Track& lhs, const Track& rhs);
    friend bool operator!=(const Track& lhs, const Track& rhs);
  protected:
    Track(Project project, std::string guid);
  private:
    static boost::optional<ChunkRegion> autoArmChunkLine(Chunk chunk);

    bool loadByGuid() const;

    void loadAndCheckIfNecessaryOrComplain() const;

    void loadIfNecessaryOrComplain() const;

    // Precondition: mediaTrack_ must be filled!
    bool isValid() const;

    boost::optional<ChunkRegion> autoArmChunkLine() const;

    // Precondition: mediaTrack_ must be filled!
    ReaProject* findContainingProject() const;

    // Precondition: mediaTrack_ must be filled!
    void attemptToFillProjectIfNecessary() const;

    Project uncheckedProject() const;

    void complainIfNotValid() const;
  };
}

