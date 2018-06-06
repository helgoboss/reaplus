#pragma once

#include <string>

namespace reaplus {
  class Tempo {
  private:
    double bpm_;
  public:
    static Tempo ofNormalizedValue(double normalizedValue);
    Tempo(double bpm);
    double normalizedValue() const;
    double bpm() const;
    std::string toString() const;
  };
}

