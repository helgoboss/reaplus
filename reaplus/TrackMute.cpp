#include "TrackMute.h"

using std::unique_ptr;

namespace reaplus {
  TrackMute::TrackMute(Track track) : track_(track) {

  }

  Track TrackMute::track() const {
    return track_;
  }

  ParameterType TrackMute::parameterType() const {
    return ParameterType::TrackMute;
  }


  bool TrackMute::equals(const Parameter& other) const {
    auto& o = static_cast<const TrackMute&>(other);
    return track_ == o.track_;
  }

  unique_ptr<Parameter> TrackMute::clone() const {
    return unique_ptr<TrackMute>(new TrackMute(*this));
  }



}

