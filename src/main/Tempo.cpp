#include <reaplus/Tempo.h>
#include <algorithm>
#include <reaplus/utility.h>
#include <helgoboss/math-util.h>

namespace reaplus {
  Tempo Tempo::ofNormalizedValue(double normalizedValue) {
    return Tempo(helgoboss::util::mapNormalizedValueToValueInRange(normalizedValue, 1, 960));
  }

  Tempo::Tempo(double bpm) {
    bpm_ = std::max(1.0, std::min(960.0, bpm));
  }

  double Tempo::normalizedValue() const {
    return helgoboss::util::mapValueInRangeToNormalizedValue(bpm_, 1, 960);
  }

  double Tempo::bpm() const {
    return bpm_;
  }

  std::string Tempo::toString() const {
    return toStringWithoutUnit() + " bpm";
  }

  std::string Tempo::toStringWithoutUnit() const {
    // @closureIsSafe
    return reaplus::toString(10, [this](char* buffer, int maxSize) {
      sprintf(buffer, "%.4f", bpm_);
    });
  }
}
