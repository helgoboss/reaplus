#pragma once

#include <string>

namespace reaplus {
  class Pan {
  private:
    double normalizedValue_;
  public:
    static Pan ofReaperValue(double reaperValue);
    static Pan ofPanExpression(const std::string& panExpression);
    Pan(double normalizedValue);
    double normalizedValue() const;
    double reaperValue() const;
    std::string toString() const;
  };
}

