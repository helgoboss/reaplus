#pragma once

#include "Parameter.h"
#include "Track.h"

namespace reaplus {
  class TrackMute: public TrackParameter {
  private:
    Track track_;
  public:
    virtual std::unique_ptr<Parameter> clone() const override;

    virtual ParameterType parameterType() const override;
    virtual bool equals(const Parameter& other) const override;

    TrackMute(Track track);
    Track track() const;
  };
}

