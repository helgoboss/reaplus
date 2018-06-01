#include "TrackSolo.h"

using std::unique_ptr;

namespace reaplus {
  TrackSolo::TrackSolo(Track track) : track_(track) {

  }

  Track TrackSolo::track() const {
    return track_;
  }

  ParameterType TrackSolo::parameterType() const {
    return ParameterType::TrackSolo;
  }


  bool TrackSolo::equals(const Parameter& other) const {
    auto& o = static_cast<const TrackSolo&>(other);
    return track_ == o.track_;
  }

  unique_ptr<Parameter> TrackSolo::clone() const {
    return unique_ptr<TrackSolo>(new TrackSolo(*this));
  }



}

