#include "TrackVolume.h"

using std::unique_ptr;

namespace reaplus {
  TrackVolume::TrackVolume(Track track) : track_(track) {

  }

  Track TrackVolume::track() const {
    return track_;
  }

  ParameterType TrackVolume::parameterType() const {
    return ParameterType::TrackVolume;
  }


  bool TrackVolume::equals(const Parameter& other) const {
    auto& o = static_cast<const TrackVolume&>(other);
    return track_ == o.track_;
  }

  unique_ptr<Parameter> TrackVolume::clone() const {
    return unique_ptr<TrackVolume>(new TrackVolume(*this));
  }



}

