#include <reaplus/TrackArm.h>
#include <memory>
using std::unique_ptr;

namespace reaplus {
  TrackArm::TrackArm(Track track) : track_(std::move(track)) {

  }

  Track TrackArm::track() const {
    return track_;
  }

  ParameterType TrackArm::parameterType() const {
    return ParameterType::TrackArm;
  }

  bool TrackArm::equals(const Parameter& other) const {
    auto& o = dynamic_cast<const TrackArm&>(other);
    return track_ == o.track_;
  }

  unique_ptr<Parameter> TrackArm::clone() const {
    return std::make_unique<TrackArm>(*this);
  }

}

