#pragma once

#include "Track.h"
#include "Volume.h"
#include "Pan.h"
#include <boost/optional.hpp>

namespace reaplus {
  class TrackSend {
    friend class Track;
  private:
    // DONE-rust
    Track sourceTrack_;
    boost::optional<Track> targetTrack_;
    mutable boost::optional<int> index_;
  public:
    // DONE-rust
    Track sourceTrack() const;
    // DONE-rust
    Track targetTrack() const;
    // DONE-rust
    int index() const;
    std::string name() const;
    // DONE-rust
    void setVolume(double normalizedValue);
    // DONE-rust
    Volume volume() const;
    // DONE-rust
    void setPan(double normalizedValue);
    // DONE-rust
    Pan pan() const;
    // DONE-rust
    bool isAvailable() const;

    // DONE-rust
    friend bool operator==(const TrackSend& lhs, const TrackSend& rhs);
  protected:
    // DONE-rust
    static Track targetTrack(Track sourceTrack, int index);
    // Use this if you want to create a target-track based send (more stable but sometimes not desired -
    // just think of presets that should work in other projects as well).
    // If you know the index, provide it as well!
    // DONE-rust
    TrackSend(Track sourceTrack, Track targetTrack, boost::optional<int> index);
    // Use this if you want to create an index-based send.
    // DONE-rust
    TrackSend(Track sourceTrack, int index);
  private:
    // DONE-rust
    static MediaTrack* targetMediaTrack(Track sourceTrack, int sendIndex);

    // DONE-rust
    bool loadByTargetTrack() const;

    // DONE-rust
    void checkOrLoadIfNecessaryOrComplain() const;

    // DONE-rust
    bool isLoadedAndAtCorrectIndex() const;

    // DONE-rust
    boost::optional<Track> targetTrackByIndex() const;

    // DONE-rust
    bool isIndexBased() const;

    // DONE-rust
    bool indexIsInRange() const;

    // DONE-rust
    bool isAtCorrectIndex() const;
  };
}
