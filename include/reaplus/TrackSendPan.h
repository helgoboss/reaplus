#pragma once

#include "Parameter.h"
#include "TrackSend.h"

namespace reaplus {
  class TrackSendPan : public TrackParameter {
  private:
    TrackSend trackSend_;
  public:
    std::unique_ptr<Parameter> clone() const override;

    Track track() const override;

    ParameterType parameterType() const override;
    bool equals(const Parameter& other) const override;

    explicit TrackSendPan(TrackSend trackSend);
    TrackSend trackSend() const;
  };
}

