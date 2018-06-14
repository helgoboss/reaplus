#include "TrackPan.h"

using std::unique_ptr;

namespace reaplus {
  TrackPan::TrackPan(Track track) : track_(track) {

  }

  Track TrackPan::track() const {
    return track_;
  }

  ParameterType TrackPan::parameterType() const {
    return ParameterType::TrackPan;
  }

  bool TrackPan::equals(const Parameter& other) const {
    auto& o = static_cast<const TrackPan&>(other);
    return track_ == o.track_;
  }

  unique_ptr<Parameter> TrackPan::clone() const {
    return unique_ptr<TrackPan>(new TrackPan(*this));
  }

}

