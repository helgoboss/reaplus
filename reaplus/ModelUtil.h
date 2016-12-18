#pragma once

#include <string>
#include <functional>

namespace reaplus {
  namespace ModelUtil {
    /**
     * Maps value in [0, 1] to [rangeMin, rangeMax] or [-1, 0] to [-rangeMax, -rangeMin].
     */
    double mapNormalizedValueToValueInRange(double value, double rangeMin, double rangeMax);

    /**
     * Maps value in [rangeMin, rangeMax] to [0, 1] or [-rangeMax, -rangeMin] to [-1, 0].
     */
    double mapValueInRangeToNormalizedValue(double value, double rangeMin, double rangeMax);

    /**
     * Maps value in [sourceRangeMin, sourceRangeMax] to [targetRangeMin, targetRangeMax] or
     * [-sourceRangeMax, -sourceRangeMin] to [-targetRangeMax, -targetRangeMin].
     */
    double mapValueInRangeToValueInRange(double value, double sourceRangeMin, double sourceRangeMax,
        double targetRangeMin, double targetRangeMax);

  }
}