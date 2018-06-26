#include "MasterPlayrate.h"
#include <memory>
using std::unique_ptr;

namespace reaplus {
  ParameterType MasterPlayrate::parameterType() const {
    return ParameterType::MasterPlayrate;
  }

  bool MasterPlayrate::equals(const Parameter& other) const {
    // TODO Shouldn't we check first if it has the correct type?
    auto& o = dynamic_cast<const MasterPlayrate&>(other);
    return true;
  }

  unique_ptr<Parameter> MasterPlayrate::clone() const {
    // TODO This parameter doesn't have any attribute, kind of pointless to copy an empty object
    return std::make_unique<MasterPlayrate>(*this);
  }

}

