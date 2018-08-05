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
    // Returns -1 if no step size (continuous character)
    double stepSize() const;
    friend bool operator==(const FxParameter& lhs, const FxParameter& rhs);
    friend bool operator!=(const FxParameter& lhs, const FxParameter& rhs);
  protected:
    FxParameter(Fx fx, int index);

  private:
    bool equals(const Parameter& other) const override;
  };
}

