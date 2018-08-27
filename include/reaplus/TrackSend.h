#pragma once

#include "Track.h"
#include "Volume.h"
#include "Pan.h"
#include <boost/optional.hpp>

namespace reaplus {
  class TrackSend {
    friend class Track;
  private:
    Track sourceTrack_;
    boost::optional<Track> targetTrack_;
    mutable boost::optional<int> index_;
  public:
    Track sourceTrack() const;
    Track targetTrack() const;
    int index() const;
    std::string name() const;
    void setVolume(double normalizedValue);
    Volume volume() const;
    void setPan(double normalizedValue);
    Pan pan() const;
    bool isAvailable() const;

    friend bool operator==(const TrackSend& lhs, const TrackSend& rhs);
  protected:
    static Track targetTrack(Track sourceTrack, int index);
    // Use this if you want to create a target-track based send (more stable but sometimes not desired -
    // just think of presets that should work in other projects as well).
    // If you know the index, provide it as well!
    TrackSend(Track sourceTrack, Track targetTrack, boost::optional<int> index);
    // Use this if you want to create an index-based send.
    TrackSend(Track sourceTrack, int index);
  private:
    static MediaTrack* targetMediaTrack(Track sourceTrack, int sendIndex);

    bool loadByTargetTrack() const;

    void checkOrLoadIfNecessaryOrComplain() const;

    bool isLoadedAndAtCorrectIndex() const;

    boost::optional<Track> targetTrackByIndex() const;

    bool isIndexBased() const;

    bool indexIsInRange() const;

    bool isAtCorrectIndex() const;
  };
}
