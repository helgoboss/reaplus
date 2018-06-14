#pragma once

#include "Parameter.h"
#include "Track.h"

namespace reaplus {
  class TrackVolume : public TrackParameter {
  private:
    Track track_;
  public:
    std::unique_ptr<Parameter> clone() const override;

    ParameterType parameterType() const override;
    bool equals(const Parameter& other) const override;

    explicit TrackVolume(Track track);
    Track track() const override;
  };
}

