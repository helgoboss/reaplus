#include "TrackSelection.h"

using std::unique_ptr;

namespace reaplus {
  TrackSelection::TrackSelection(Track track) : track_(track) {

  }

  Track TrackSelection::track() const {
    return track_;
  }

  ParameterType TrackSelection::parameterType() const {
    return ParameterType::TrackSelection;
  }

  bool TrackSelection::equals(const Parameter& other) const {
    auto& o = static_cast<const TrackSelection&>(other);
    return track_ == o.track_;
  }

  unique_ptr<Parameter> TrackSelection::clone() const {
    return unique_ptr<TrackSelection>(new TrackSelection(*this));
  }

}

