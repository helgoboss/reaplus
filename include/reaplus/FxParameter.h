#pragma once

#include <string>
#include "Fx.h"
#include "Parameter.h"

namespace reaplus {
  enum class FxParameterCharacter {
    Toggle,
    Discrete,
    Continuous
  };

  struct FxParameterValueRange {
    double minVal = -1;
    double midVal = -1;
    double maxVal = -1;
  };

  class FxParameter : public TrackParameter {
    friend class Fx;
  private:
    Fx fx_;
    int index_;
  public:

    std::unique_ptr<Parameter> clone() const override;

    Track track() const override;

    ParameterType parameterType() const override;

    int index() const;
    std::string name() const;
    std::string formattedValue() const;
    bool isAvailable() const;
    // Returns normalized value [0, 1]
    double normalizedValue() const;
    double reaperValue() const;
    void setNormalizedValue(double normalizedValue);
    std::string formatNormalizedValue(double normalizedValue) const;
    Fx fx() const;
    FxParameterCharacter character() const;
    // Returns a normalized value
    // Returns -1 if no step size (continuous character)
    // TODO This is a too opinionated function in that it already interprets and processes some of REAPER's return
    // values.
    double stepSize() const;
    // Doesn't necessarily return normalized values
    FxParameterValueRange valueRange() const;
    friend bool operator==(const FxParameter& lhs, const FxParameter& rhs);
    friend bool operator!=(const FxParameter& lhs, const FxParameter& rhs);
  protected:
    FxParameter(Fx fx, int index);

  private:
    bool equals(const Parameter& other) const override;
  };
}

