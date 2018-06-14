#pragma once

#include "Parameter.h"

namespace reaplus {
  class MasterTempo : public Parameter {
  public:
    virtual std::unique_ptr<Parameter> clone() const override;

    virtual ParameterType parameterType() const override;
    virtual bool equals(const Parameter& other) const override;
  };
}

