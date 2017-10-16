#include "TrackArm.h"

using std::unique_ptr;

namespace reaplus {
  TrackArm::TrackArm(Track track) : track_(track) {

  }

  Track TrackArm::track() const {
    return track_;
  }

  ParameterType TrackArm::parameterType() const {
    return ParameterType::TrackArm;
  }


  bool TrackArm::equals(const Parameter& other) const {
    auto& o = static_cast<const TrackArm&>(other);
    return track_ == o.track_;
  }

  unique_ptr<Parameter> TrackArm::clone() const {
    return unique_ptr<TrackArm>(new TrackArm(*this));
  }



}

