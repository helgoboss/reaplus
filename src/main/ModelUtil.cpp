#include <reaplus/ModelUtil.h>
#include <cmath>

using std::string;
using std::function;

namespace reaplus {
  namespace ModelUtil {
    double mapNormalizedValueToValueInRange(double value, double rangeMin, double rangeMax) {
      const double targetSpan = rangeMax - rangeMin;
      return (std::signbit(value) ? -1 : 1) * (rangeMin + std::abs(value) * targetSpan);
    }

    // -1   -1    1 => 0
    // -0.5 -1    1 => 0.25
    // 0    -1    1 => 0.5
    // 0.5  -1    1 => 0.75
    // 1    -1    1 => 1

    // -1   0     1 => -1
    // -0.5 0     1 => -0.5
    // 0    0     1 => 0
    // 0.5  0     1 => 0.5
    // 1    0     1 => 1

    // -2   1     2 => -1
    // -1.5 1     2 => -0.5
    // -1   1     2 => 0
    // 1    1     2 => 0
    // 1.5  1     2 => 0.5
    // 2    1     2 => 1
    double mapValueInRangeToNormalizedValue(double value, double rangeMin, double rangeMax) {
      const double sourceSpan = rangeMax - rangeMin;
      if (sourceSpan == 0.0) {
        return 0.0;
      } else {
        if (rangeMin < 0) {
          const double positiveValue = value + std::abs(rangeMin);
          return positiveValue / sourceSpan;
        } else {
          return (std::signbit(value) ? -1 : 1) * ((std::abs(value) - rangeMin) / sourceSpan);
        }
      }
    }

    double mapValueInRangeToValueInRange(double value, double sourceRangeMin, double sourceRangeMax,
        double targetRangeMin, double targetRangeMax) {
      const auto positiveValue = std::abs(value);
      if (positiveValue < sourceRangeMin || positiveValue > sourceRangeMax) {
        return 0.0;
      } else {
        return mapNormalizedValueToValueInRange(
            mapValueInRangeToNormalizedValue(value, sourceRangeMin, sourceRangeMax),
            targetRangeMin,
            targetRangeMax
        );
      }
    }

  }
}
