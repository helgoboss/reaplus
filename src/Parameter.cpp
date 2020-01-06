#include <reaplus/Parameter.h>
#include <typeinfo>

using std::shared_ptr;

namespace reaplus {

  bool operator==(const Parameter& lhs, const Parameter& rhs) {
    return typeid(lhs) == typeid(rhs) && lhs.equals(rhs);
  }

  bool Parameter::isTrackParameter() const {
    return false;
  }

  bool TrackParameter::isTrackParameter() const {
    return true;
  }

}