#pragma once

#include <reaper_plugin.h>
#include <string>
#include <functional>
#include <memory>
#include <rxcpp/rx.hpp>
#include <boost/optional.hpp>

namespace reaplus {
  class Track;
  class Project {
  private:
    ReaProject* reaProject_;
  public:
    Project(ReaProject* reaProject);
    int index() const;
    int trackCount() const;
    bool isAvailable() const;
    rxcpp::observable<Track> tracks() const;
    int selectedTrackCount() const;
    rxcpp::observable<Track> selectedTracks() const;
    boost::optional<Track> firstSelectedTrack() const;
    // This returns a non-optional in order to support not-yet-loaded tracks. GUID is a perfectly stable
    // identifier of a track!
    Track trackByGuid(const std::string& guid) const;
    // It's correct that this returns an optional because the index isn't a stable identifier of a track.
    // The track could move. So this should do a runtime lookup of the track and return a stable MediaTrack-backed
    // Track object if a track exists at that index.
    boost::optional<Track> trackByIndex(int index) const;
    Track masterTrack() const;
    boost::optional<Track> firstTrack() const;
    boost::optional<Track> lastTrack() const;
    ReaProject* reaProject() const;
    Track addTrack();
    Track insertTrackAt(int index);
    void removeTrack(Track track);
    boost::optional<std::string> filePath() const;
    friend bool operator==(const Project& lhs, const Project& rhs);
    friend bool operator!=(const Project& lhs, const Project& rhs);
  private:
    void complainIfNotAvailable() const;
  };
}

