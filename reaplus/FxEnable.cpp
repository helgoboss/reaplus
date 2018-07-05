#include "FxEnable.h"
using std::unique_ptr;

namespace reaplus {
  FxEnable::FxEnable(Fx fx) : fx_(std::move(fx)) {
  }

  Track FxEnable::track() const {
    return fx_.track();
  }

  ParameterType FxEnable::parameterType() const {
    return ParameterType::FxEnable;
  }

  bool FxEnable::equals(const Parameter& other) const {
    auto& o = dynamic_cast<const FxEnable&>(other);
    return fx_ == o.fx_;
  }

  unique_ptr<Parameter> FxEnable::clone() const {
    return std::make_unique<FxEnable>(*this);
  }
  Fx FxEnable::fx() const {
    return fx_;
  }

}

