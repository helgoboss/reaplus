#pragma once

#include <string>

namespace reaplus {
  class Pan {
  private:
    // DONE-rust
    double normalizedValue_;
  public:
    // DONE-rust
    static Pan ofReaperValue(double reaperValue);
    // TODO-rust
    static Pan ofPanExpression(const std::string& panExpression);
    // DONE-rust
    explicit Pan(double normalizedValue);
    // DONE-rust
    double normalizedValue() const;
    // DONE-rust
    double reaperValue() const;
    // TODO-rust
    std::string toString() const;
  };
}

