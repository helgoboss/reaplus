#pragma once

#include "Track.h"
#include <memory>

namespace reaplus {
  enum class ParameterType {
    FX,
    TrackVolume,
    TrackSendVolume,
    Action,
    TrackPan,
    TrackArm,
    TrackSelection,
    TrackMute,
    TrackSolo,
    TrackSendPan,
    MasterTempo
  };

  class Parameter {
  public:
    virtual ParameterType parameterType() const = 0;
    friend bool operator==(const Parameter& lhs, const Parameter& rhs);
    virtual bool isTrackParameter() const;
    virtual std::unique_ptr<Parameter> clone() const = 0;
  private:
    virtual bool equals(const Parameter& other) const = 0;
  };

  // FIXME Factor out into separate file
  class TrackParameter : public Parameter {
  public:
    virtual bool isTrackParameter() const override;

    virtual Track track() const = 0;
  };
}

