#pragma once

#include <string>

namespace reaplus {
  // TODO-rust
  class Playrate {
  private:
    double value_;
  public:
    static Playrate ofNormalizedValue(double normalizedValue);
    explicit Playrate(double value);
    double normalizedValue() const;
    double value() const;
    std::string toString() const;
    std::string toStringWithoutUnit() const;
  };
}

