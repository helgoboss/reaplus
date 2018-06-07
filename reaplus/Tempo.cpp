#include "Tempo.h"
#include "helgoboss/utility.h"
#include "ModelUtil.h"

namespace reaplus {
  Tempo Tempo::ofNormalizedValue(double normalizedValue) {
    return Tempo(ModelUtil::mapNormalizedValueToValueInRange(normalizedValue, 1, 960));
  }

  Tempo::Tempo(double bpm) {
    bpm_ = bpm;
  }

  double Tempo::normalizedValue() const {
    return ModelUtil::mapValueInRangeToNormalizedValue(bpm_, 1, 960);
  }

  double Tempo::bpm() const {
    return bpm_;
  }

  std::string Tempo::toString() const {
    // @closureIsSafe
    return helgoboss::toString(12, [this](char* buffer, int maxSize) {
      sprintf(buffer, "%.2f bpm", bpm_);
    });
  }
}
