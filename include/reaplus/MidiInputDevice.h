#pragma once

#include <string>

namespace reaplus {
  // DONE-rust
  class MidiInputDevice {
  private:
    int id_;
  public:
    // DONE-rust
    explicit MidiInputDevice(int id);

    // DONE-rust
    int id() const;

    // DONE-rust
    std::string name() const;

    // For REAPER < 5.94 this is the same like isConnected(). For REAPER >=5.94 it returns true if the device ever
    // existed, even if it's disconnected now.
    // DONE-rust
    bool isAvailable() const;

    // Only returns true if the device is connected (= present)
    // DONE-rust
    bool isConnected() const;

    // DONE-rust
    friend bool operator==(const MidiInputDevice& lhs, const MidiInputDevice& rhs);

    // DONE-rust
    friend bool operator!=(const MidiInputDevice& lhs, const MidiInputDevice& rhs);
  };
}

