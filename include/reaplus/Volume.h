#pragma once

#include <string>

namespace reaplus {
  class Volume {
  private:
    // DONE-rust
    double normalizedValue_;
  public:
    // DONE-rust
    static Volume ofReaperValue(double reaperValue);
    // DONE-rust
    static Volume ofDb(double db);
    // DONE-rust
    explicit Volume(double normalizedValue);
    // DONE-rust
    double normalizedValue() const;
    // DONE-rust
    double reaperValue() const;
    // DONE-rust
    double db() const;
    std::string toString() const;
    std::string toStringWithoutUnit() const;
  };
}

