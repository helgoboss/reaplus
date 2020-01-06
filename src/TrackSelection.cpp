#include <reaplus/TrackSelection.h>
#include <memory>
#include <utility>
using std::unique_ptr;

namespace reaplus {
  TrackSelection::TrackSelection(Track track) : track_(std::move(track)) {

  }

  Track TrackSelection::track() const {
    return track_;
  }

  ParameterType TrackSelection::parameterType() const {
    return ParameterType::TrackSelection;
  }

  bool TrackSelection::equals(const Parameter& other) const {
    auto& o = dynamic_cast<const TrackSelection&>(other);
    return track_ == o.track_;
  }

  unique_ptr<Parameter> TrackSelection::clone() const {
    return std::make_unique<TrackSelection>(*this);
  }

}

