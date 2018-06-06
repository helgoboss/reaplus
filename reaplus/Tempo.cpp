#include "Tempo.h"
#include "helgoboss/utility.h"

namespace reaplus {
  Tempo Tempo::ofNormalizedValue(double normalizedValue) {
    return Tempo(normalizedValue * 960);
  }

  Tempo::Tempo(double bpm) {
    bpm_ = bpm;
  }

  double Tempo::normalizedValue() const {
    return bpm_ / 960;
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
