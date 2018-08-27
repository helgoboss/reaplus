#pragma once

#include <string>

namespace reaplus {
  class Volume {
  private:
    double normalizedValue_;
  public:
    static Volume ofReaperValue(double reaperValue);
    static Volume ofDb(double db);
    explicit Volume(double normalizedValue);
    double normalizedValue() const;
    double reaperValue() const;
    double db() const;
    std::string toString() const;
    std::string toStringWithoutUnit() const;
  };
}

