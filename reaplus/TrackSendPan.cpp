#include "TrackSendPan.h"

using std::unique_ptr;

namespace reaplus {

  TrackSendPan::TrackSendPan(TrackSend trackSend) : trackSend_(trackSend) {

  }

  TrackSend TrackSendPan::trackSend() const {
    return trackSend_;
  }

  ParameterType TrackSendPan::parameterType() const {
    return ParameterType::TrackSendPan;
  }

  bool TrackSendPan::equals(const Parameter& other) const {
    auto& o = static_cast<const TrackSendPan&>(other);
    return trackSend_ == o.trackSend_;
  }

  Track TrackSendPan::track() const {
    return trackSend_.sourceTrack();
  }

  unique_ptr<Parameter> TrackSendPan::clone() const {
    return unique_ptr<TrackSendPan>(new TrackSendPan(*this));
  }

}