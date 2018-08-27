#pragma once

#include "Parameter.h"

namespace reaplus {
  class MasterTempo : public Parameter {
  public:
    std::unique_ptr<Parameter> clone() const override;

    ParameterType parameterType() const override;
    bool equals(const Parameter& other) const override;
  };
}

