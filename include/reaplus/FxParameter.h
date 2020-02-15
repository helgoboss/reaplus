#pragma once

#include <string>
#include "Fx.h"
#include "Parameter.h"

namespace reaplus {
  // DONE-rust
  enum class FxParameterCharacter {
    Toggle,
    Discrete,
    Continuous
  };

  // DONE-rust
  struct FxParameterValueRange {
    double minVal = -1;
    double midVal = -1;
    double maxVal = -1;
  };

  class FxParameter : public TrackParameter {
    friend class Fx;
  private:
    // DONE-rust
    Fx fx_;
    int index_;
  public:

    std::unique_ptr<Parameter> clone() const override;

    // DONE-rust
    Track track() const override;

    ParameterType parameterType() const override;

    // DONE-rust
    int index() const;
    // DONE-rust
    std::string name() const;
    // DONE-rust
    std::string formattedValue() const;
    // DONE-rust
    bool isAvailable() const;
    // Returns normalized value [0, 1]
    // DONE-rust
    double normalizedValue() const;
    // DONE-rust
    double reaperValue() const;
    // TODO-rust
    void setNormalizedValue(double normalizedValue);
    // DONE-rust
    std::string formatNormalizedValue(double normalizedValue) const;
    // DONE-rust
    Fx fx() const;
    // DONE-rust
    FxParameterCharacter character() const;
    // Returns a normalized value
    // Returns -1 if no step size (continuous character)
    // TODO This is a too opinionated function in that it already interprets and processes some of REAPER's return
    // values.
    // DONE-rust
    double stepSize() const;
    // Doesn't necessarily return normalized values
    // DONE-rust
    FxParameterValueRange valueRange() const;
    // DONE-rust
    friend bool operator==(const FxParameter& lhs, const FxParameter& rhs);
    // DONE-rust
    friend bool operator!=(const FxParameter& lhs, const FxParameter& rhs);
  protected:
    // DONE-rust
    FxParameter(Fx fx, int index);

  private:
    bool equals(const Parameter& other) const override;
  };
}

