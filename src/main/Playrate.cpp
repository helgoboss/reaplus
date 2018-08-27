#include <reaplus/Playrate.h>
#include <algorithm>
#include <reaplus/utility.h>
#include <reaplus/ModelUtil.h>
#include <reaper_plugin_functions.h>

namespace reaplus {
  Playrate Playrate::ofNormalizedValue(double normalizedValue) {
    return Playrate(reaper::Master_NormalizePlayRate(normalizedValue, true));
  }

  Playrate::Playrate(double value) {
    value_ = value;
  }

  double Playrate::normalizedValue() const {
    return reaper::Master_NormalizePlayRate(value_, false);
  }

  double Playrate::value() const {
    return value_;
  }

  std::string Playrate::toString() const {
    return toStringWithoutUnit() + " x";
  }

  std::string Playrate::toStringWithoutUnit() const {
    // @closureIsSafe
    return reaplus::toString(8, [this](char* buffer, int maxSize) {
      sprintf(buffer, "%.2f", value_);
    });
  }
}
