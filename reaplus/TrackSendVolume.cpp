#include "TrackSendVolume.h"
#include <memory> #include <utility>
using std::unique_ptr;

namespace reaplus {

  TrackSendVolume::TrackSendVolume(TrackSend trackSend) : trackSend_(std::move(trackSend)) {

  }

  TrackSend TrackSendVolume::trackSend() const {
    return trackSend_;
  }

  ParameterType TrackSendVolume::parameterType() const {
    return ParameterType::TrackSendVolume;
  }

  bool TrackSendVolume::equals(const Parameter& other) const {
    auto& o = dynamic_cast<const TrackSendVolume&>(other);
    return trackSend_ == o.trackSend_;
  }

  Track TrackSendVolume::track() const {
    return trackSend_.sourceTrack();
  }

  unique_ptr<Parameter> TrackSendVolume::clone() const {
    return std::make_unique<TrackSendVolume>(*this);
  }

}