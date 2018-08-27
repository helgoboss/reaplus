#include <reaplus/TrackSolo.h>
#include <memory>
#include <utility>
using std::unique_ptr;

namespace reaplus {
  TrackSolo::TrackSolo(Track track) : track_(std::move(track)) {

  }

  Track TrackSolo::track() const {
    return track_;
  }

  ParameterType TrackSolo::parameterType() const {
    return ParameterType::TrackSolo;
  }

  bool TrackSolo::equals(const Parameter& other) const {
    auto& o = dynamic_cast<const TrackSolo&>(other);
    return track_ == o.track_;
  }

  unique_ptr<Parameter> TrackSolo::clone() const {
    return std::make_unique<TrackSolo>(*this);
  }

}

