#include "TrackSend.h"
#include "ModelUtil.h"
#include "utility.h"
#include "HelperControlSurface.h"

#include "reaper_plugin_functions.h"

using std::string;

namespace reaplus {

  TrackSend::TrackSend(Track sourceTrack, int index): sourceTrack_(sourceTrack), targetTrack_(boost::none), index_(index) {
  }

  Track TrackSend::sourceTrack() const {
    return sourceTrack_;
  }

  int TrackSend::index() const {
    checkOrLoadIfNecessaryOrComplain();
    return *index_;
  }

  string TrackSend::name() const {
    return reaplus::toString(256, [this](char* buffer, int maxSize) {
      reaper::GetTrackSendName(sourceTrack().mediaTrack(), index(), buffer, maxSize);
    });
  }

  void TrackSend::setVolume(double normalizedValue) {
    const double reaperValue = Volume(normalizedValue).reaperValue();
    reaper::CSurf_OnSendVolumeChange(sourceTrack().mediaTrack(), index(), reaperValue, false);
  }

  Volume TrackSend::volume() const {
    // It's important that we don't use GetTrackSendInfo_Value with D_VOL because it returns the wrong value if
    // an envelope is written.
    double volume;
    reaper::GetTrackSendUIVolPan(sourceTrack().mediaTrack(), index(), &volume, nullptr);
    return Volume::ofReaperValue(volume);
  }

  void TrackSend::setPan(double normalizedValue) {
    const double reaperValue = Pan(normalizedValue).reaperValue();
    reaper::CSurf_OnSendPanChange(sourceTrack().mediaTrack(), index(), reaperValue, false);
  }

  Pan TrackSend::pan() const {
    double pan;
    reaper::GetTrackSendUIVolPan(sourceTrack().mediaTrack(), index(), nullptr, &pan);
    return Pan::ofReaperValue(pan);
  }

  bool operator==(const TrackSend& lhs, const TrackSend& rhs) {
    if (lhs.sourceTrack_ == rhs.sourceTrack_) {
      if (lhs.isIndexBased() && rhs.isIndexBased()) {
        return *lhs.index_ == *rhs.index_;
      } else if (!lhs.isIndexBased() && !rhs.isIndexBased()) {
        return *lhs.targetTrack_ == *rhs.targetTrack_;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  Track TrackSend::targetTrack(Track sourceTrack, int index) {
    return Track(targetMediaTrack(sourceTrack, index), sourceTrack.project().reaProject());
  }

  MediaTrack* TrackSend::targetMediaTrack(Track sourceTrack, int sendIndex) {
    auto targetMediaTrack = (MediaTrack*) reaper::GetSetTrackSendInfo(sourceTrack.mediaTrack(), 0, sendIndex, "P_DESTTRACK", nullptr);
    return targetMediaTrack;
  }

  bool TrackSend::loadByTargetTrack() const {
    if (targetTrack_) {
      // Precondition: targetTrack is set
      if (!sourceTrack_.isAvailable()) {
        return false;
      }
      const auto foundTrackSend = sourceTrack().sends()
          .filter([this](TrackSend s) {
            return s.targetTrack() == *targetTrack_;
          })
          .map([](TrackSend s) {
            return boost::make_optional(s);
          })
          .default_if_empty((boost::optional<TrackSend>) boost::none)
          .as_blocking()
          .first();
      if (foundTrackSend) {
        index_ = foundTrackSend->index();
        return true;
      } else {
        return false;
      }
    } else {
      return false;
    }
  }

  void TrackSend::checkOrLoadIfNecessaryOrComplain() const {
    if (isIndexBased()) {
      // Index based
      if (!indexIsInRange()) {
        throw std::logic_error("Index based send not loadable");
      }
    } else {
      // Target track based
      if (!isLoadedAndAtCorrectIndex() && !loadByTargetTrack()) {
        throw std::logic_error("Target track based send not loadable");
      }
    }
  }


  bool TrackSend::isIndexBased() const {
    return !targetTrack_.is_initialized();
  }

  bool TrackSend::isLoadedAndAtCorrectIndex() const {
    // Precondition: is target track based
    if (index_) {
      return isAtCorrectIndex();
    } else {
      // Not loaded
      return false;
    }
  }

  bool TrackSend::isAtCorrectIndex() const {
    // Precondition: is target track based
    return sourceTrack_.isAvailable() && targetTrackByIndex() == targetTrack_;
  }

  bool TrackSend::indexIsInRange() const {
    return sourceTrack_.isAvailable() && *index_ < sourceTrack_.sendCount();
  }

  Track TrackSend::targetTrack() const {
    if (isIndexBased()) {
      return TrackSend::targetTrack(sourceTrack_, index());
    } else {
      return *targetTrack_;
    }
  }

  bool TrackSend::isAvailable() const {
    if (isIndexBased()) {
      return indexIsInRange();
    } else {
      if (isLoadedAndAtCorrectIndex()) {
        // Loaded and at correct index
        return true;
      } else {
        // Not yet loaded or at wrong index
        return loadByTargetTrack();
      }
    }
  }

  TrackSend::TrackSend(Track sourceTrack, Track targetTrack, boost::optional<int> index) : sourceTrack_(sourceTrack),
      targetTrack_(targetTrack), index_(index)  {

  }

  boost::optional<Track> TrackSend::targetTrackByIndex() const {
    // Precondition: index set
    auto targetMediaTrack = TrackSend::targetMediaTrack(sourceTrack_, *index_);
    if (targetMediaTrack == nullptr) {
      return boost::none;
    } else {
      return Track(targetMediaTrack, sourceTrack_.project().reaProject());
    }
  }

}