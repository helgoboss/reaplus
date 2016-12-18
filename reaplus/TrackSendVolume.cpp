#include "TrackSendVolume.h"

using std::unique_ptr;

namespace reaplus {

  TrackSendVolume::TrackSendVolume(TrackSend trackSend): trackSend_(trackSend) {

  }

  TrackSend TrackSendVolume::trackSend() const {
    return trackSend_;
  }


  ParameterType TrackSendVolume::parameterType() const {
    return ParameterType::TrackSendVolume;
  }


  bool TrackSendVolume::equals(const Parameter& other) const {
    auto& o = static_cast<const TrackSendVolume&>(other);
    return trackSend_ == o.trackSend_;
  }

  Track TrackSendVolume::track() const {
    return trackSend_.sourceTrack();
  }


  unique_ptr<Parameter> TrackSendVolume::clone() const {
    return unique_ptr<TrackSendVolume>(new TrackSendVolume(*this));
  }



}