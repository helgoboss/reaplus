#include "MasterTempo.h"

using std::unique_ptr;

namespace reaplus {
  ParameterType MasterTempo::parameterType() const {
    return ParameterType::MasterTempo;
  }

  bool MasterTempo::equals(const Parameter& other) const {
    // TODO Shouldn't we check first if it has the correct type?
    auto& o = static_cast<const MasterTempo&>(other);
    return true;
  }

  unique_ptr<Parameter> MasterTempo::clone() const {
    // TODO This parameter doesn't have any attribute, kind of pointless to copy an empty object
    return unique_ptr<MasterTempo>(new MasterTempo(*this));
  }

}

