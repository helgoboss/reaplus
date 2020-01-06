#include <reaplus/Pan.h>
#include <cmath>
#include <helgoboss-learn/math-util.h>
#include <reaplus/utility.h>

#include <reaper_plugin_functions.h>

namespace reaplus {
  Pan::Pan(double normalizedValue) : normalizedValue_(normalizedValue) {
  }

  Pan Pan::ofReaperValue(double reaperValue) {
    return Pan(helgoboss::util::mapValueInRangeToNormalizedValue(reaperValue, -1, 1));
  }

  Pan Pan::ofPanExpression(const std::string& panExpression) {
    const double reaperValue = reaper::parsepanstr(panExpression.c_str());
    return Pan::ofReaperValue(reaperValue);
  }

  double Pan::reaperValue() const {
    return helgoboss::util::mapNormalizedValueToValueInRange(normalizedValue_, -1, 1);
  }

  std::string Pan::toString() const {
    const double val = reaperValue();
    return reaplus::toString(20, [val](char* buffer, int maxSize) {
      reaper::mkpanstr(buffer, val);
    });
  }

  double Pan::normalizedValue() const {
    return normalizedValue_;
  }
}

