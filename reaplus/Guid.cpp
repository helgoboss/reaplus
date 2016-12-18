#include "Guid.h"
#include "utility.h"

namespace reaplus {
  Guid::Guid(GUID data) : data_(data) {
  }

  GUID Guid::data() const {
    return data_;
  }

  std::string Guid::toString() const {
    return convertGuidToString(data_);
  }
}