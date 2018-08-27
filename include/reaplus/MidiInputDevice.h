#pragma once

#include <string>

namespace reaplus {
  class MidiInputDevice {
  private:
    int id_;
  public:
    explicit MidiInputDevice(int id);

    int id() const;

    std::string name() const;

    // For REAPER < 5.94 this is the same like isConnected(). For REAPER >=5.94 it returns true if the device ever
    // existed, even if it's disconnected now.
    bool isAvailable() const;

    // Only returns true if the device is connected (= present)
    bool isConnected() const;

    friend bool operator==(const MidiInputDevice& lhs, const MidiInputDevice& rhs);

    friend bool operator!=(const MidiInputDevice& lhs, const MidiInputDevice& rhs);
  };
}

