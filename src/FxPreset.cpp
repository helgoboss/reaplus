#include <reaplus/FxPreset.h>
using std::unique_ptr;

namespace reaplus {
  std::unique_ptr<Parameter> FxPreset::clone() const {
    return std::make_unique<FxPreset>(*this);
  }
  ParameterType FxPreset::parameterType() const {
    return ParameterType::FxEnable;
  }
  bool FxPreset::equals(const Parameter& other) const {
    auto& o = dynamic_cast<const FxPreset&>(other);
    return fx_ == o.fx_;
  }
  FxPreset::FxPreset(Fx fx) : fx_(std::move(fx)) {
  }
  Track FxPreset::track() const {
    return fx_.track();
  }
  Fx FxPreset::fx() const {
    return fx_;
  }
}