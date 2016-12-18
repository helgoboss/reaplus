#pragma once

#include <string>

namespace reaplus {
  class Pan {
  private:
    double normalizedValue_;
  public:
    static Pan ofReaperValue(double reaperValue);
    Pan(double normalizedValue);
    double normalizedValue() const;
    double reaperValue() const;
    std::string toString() const;
  };
}

