#pragma once

#include <reaper_plugin.h>
#include <string>
#include <functional>
#include <memory>
#include <rxcpp/rx.hpp>
#include <boost/optional.hpp>
#include <helgoboss-learn/Tempo.h>
#include "Playrate.h"

namespace reaplus {
  class Track;
  class Project {
  private:
    // DONE-rust
    ReaProject* reaProject_;
  public:
    // DONE-rust
    explicit Project(ReaProject* reaProject);
    // DONE-rust
    int index() const;
    // DONE-rust
    int trackCount() const;
    // DONE-rust
    bool isAvailable() const;
    // DONE-rust
    rxcpp::observable<Track> tracks() const;
    // DONE-rust
    int selectedTrackCount(bool wantMaster = false) const;
    // DONE-rust
    rxcpp::observable<Track> selectedTracks(bool wantMaster = false) const;
    // DONE-rust
    boost::optional<Track> firstSelectedTrack(bool wantMaster = false) const;
    // DONE-rust
    void unselectAllTracks();
    // This returns a non-optional in order to support not-yet-loaded tracks. GUID is a perfectly stable
    // identifier of a track!
    // DONE-rust
    Track trackByGuid(const std::string& guid) const;
    // It's correct that this returns an optional because the index isn't a stable identifier of a track.
    // The track could move. So this should do a runtime lookup of the track and return a stable MediaTrack-backed
    // Track object if a track exists at that index.
    // 0 is first normal track (master track is not obtainable via this method)
    // DONE-rust
    boost::optional<Track> trackByIndex(int index) const;
    // 0 is master track, 1 is first normal track
    // DONE-rust
    boost::optional<Track> trackByNumber(int number) const;
    // DONE-rust
    Track masterTrack() const;
    // DONE-rust
    boost::optional<Track> firstTrack() const;
    // TODO-rust
    boost::optional<Track> lastTrack() const;
    // DONE-rust
    ReaProject* reaProject() const;
    // DONE-rust
    // TODO Introduce variant that doesn't notify ControlSurface
    Track addTrack();
    // DONE-rust
    // TODO Introduce variant that doesn't notify ControlSurface
    // DONE-rust
    Track insertTrackAt(int index);
    // DONE-rust
    void removeTrack(Track track);
    // DONE-rust
    boost::optional<std::string> filePath() const;
    // DONE-rust
    helgoboss::Tempo tempo() const;
    // DONE-rust
    void setTempo(double bpm, bool wantUndo);
    // TODO-rust
    Playrate playrate() const;
    // TODO-rust
    void setPlayrate(double playrate);
    // DONE-rust
    void markAsDirty();
    // DONE-rust
    void undoable(const std::string& label, std::function<void(void)> command);
    // DONE-rust
    void undo();
    // DONE-rust
    void redo();
    // DONE-rust
    boost::optional<std::string> labelOfLastUndoableAction() const;
    // DONE-rust
    boost::optional<std::string> labelOfNextRedoableAction() const;
    // DONE-rust
    friend bool operator==(const Project& lhs, const Project& rhs);
    // DONE-rust
    friend bool operator!=(const Project& lhs, const Project& rhs);
  private:
    // DONE-rust
    void complainIfNotAvailable() const;
  };
}

