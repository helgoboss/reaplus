#pragma once

#include "Parameter.h"
#include "Fx.h"

namespace reaplus {
  // TODO-rust
  class FxPreset : public TrackParameter {
  private:
    Fx fx_;
  public:
    std::unique_ptr<Parameter> clone() const override;

    ParameterType parameterType() const override;
    bool equals(const Parameter& other) const override;

    explicit FxPreset(Fx fx);
    Track track() const override;

    Fx fx() const;
  };
}

