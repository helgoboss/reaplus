#include <reaplus/TrackMute.h>
#include <memory>
#include <utility>
using std::unique_ptr;

namespace reaplus {
  TrackMute::TrackMute(Track track) : track_(std::move(track)) {

  }

  Track TrackMute::track() const {
    return track_;
  }

  ParameterType TrackMute::parameterType() const {
    return ParameterType::TrackMute;
  }

  bool TrackMute::equals(const Parameter& other) const {
    auto& o = dynamic_cast<const TrackMute&>(other);
    return track_ == o.track_;
  }

  unique_ptr<Parameter> TrackMute::clone() const {
    return std::make_unique<TrackMute>(*this);
  }

}

