#pragma once

#include <string>

namespace reaplus {
  class Volume {
  private:
    static const double LN10_OVER_TWENTY;
    double normalizedValue_;
  public:
    static Volume ofReaperValue(double reaperValue);
    Volume(double normalizedValue);
    double normalizedValue() const;
    double reaperValue() const;
    double db() const;
    std::string toString() const;
  };
}

