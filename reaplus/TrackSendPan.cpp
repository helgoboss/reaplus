#include "TrackSendPan.h"
#include <memory> #include <utility>
using std::unique_ptr;

namespace reaplus {

  TrackSendPan::TrackSendPan(TrackSend trackSend) : trackSend_(std::move(trackSend)) {

  }

  TrackSend TrackSendPan::trackSend() const {
    return trackSend_;
  }

  ParameterType TrackSendPan::parameterType() const {
    return ParameterType::TrackSendPan;
  }

  bool TrackSendPan::equals(const Parameter& other) const {
    auto& o = dynamic_cast<const TrackSendPan&>(other);
    return trackSend_ == o.trackSend_;
  }

  Track TrackSendPan::track() const {
    return trackSend_.sourceTrack();
  }

  unique_ptr<Parameter> TrackSendPan::clone() const {
    return std::make_unique<TrackSendPan>(*this);
  }

}