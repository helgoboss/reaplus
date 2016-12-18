#pragma once

#include <string>

namespace reaplus {
  class MidiInputDevice {
  private:
    int id_;
  public:
    MidiInputDevice(int id);

    int id() const;

    std::string name() const;

    bool isAvailable() const;

    friend bool operator==(const MidiInputDevice& lhs, const MidiInputDevice& rhs);

    friend bool operator!=(const MidiInputDevice& lhs, const MidiInputDevice& rhs);
  };
}

