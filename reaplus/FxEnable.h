#pragma once

#include "Parameter.h"
#include "Fx.h"

namespace reaplus {
  class FxEnable : public TrackParameter {
  private:
    Fx fx_;
  public:
    std::unique_ptr<Parameter> clone() const override;

    ParameterType parameterType() const override;
    bool equals(const Parameter& other) const override;

    explicit FxEnable(Fx fx);
    Track track() const override;

    Fx fx() const;
  };
}

