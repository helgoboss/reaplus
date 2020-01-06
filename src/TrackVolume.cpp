#include <reaplus/TrackVolume.h>
#include <memory>
#include <utility>
using std::unique_ptr;

namespace reaplus {
  TrackVolume::TrackVolume(Track track) : track_(std::move(track)) {

  }

  Track TrackVolume::track() const {
    return track_;
  }

  ParameterType TrackVolume::parameterType() const {
    return ParameterType::TrackVolume;
  }

  bool TrackVolume::equals(const Parameter& other) const {
    auto& o = dynamic_cast<const TrackVolume&>(other);
    return track_ == o.track_;
  }

  unique_ptr<Parameter> TrackVolume::clone() const {
    return std::make_unique<TrackVolume>(*this);
  }

}

