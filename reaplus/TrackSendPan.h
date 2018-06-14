#pragma once

#include "Parameter.h"
#include "TrackSend.h"

namespace reaplus {
  class TrackSendPan : public TrackParameter {
  private:
    TrackSend trackSend_;
  public:
    virtual std::unique_ptr<Parameter> clone() const override;

    virtual Track track() const override;

    virtual ParameterType parameterType() const override;
    virtual bool equals(const Parameter& other) const override;

    TrackSendPan(TrackSend trackSend);
    TrackSend trackSend() const;
  };
}

