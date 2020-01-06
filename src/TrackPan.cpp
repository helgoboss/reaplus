#include <reaplus/TrackPan.h>
#include <memory>
#include <utility>
using std::unique_ptr;

namespace reaplus {
  TrackPan::TrackPan(Track track) : track_(std::move(track)) {

  }

  Track TrackPan::track() const {
    return track_;
  }

  ParameterType TrackPan::parameterType() const {
    return ParameterType::TrackPan;
  }

  bool TrackPan::equals(const Parameter& other) const {
    auto& o = dynamic_cast<const TrackPan&>(other);
    return track_ == o.track_;
  }

  unique_ptr<Parameter> TrackPan::clone() const {
    return std::make_unique<TrackPan>(*this);
  }

}

