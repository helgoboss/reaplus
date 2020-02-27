#pragma once

#include <reaper_plugin.h>
#include <string>
#include <helgoboss-midi/MidiMessage.h>

namespace reaplus {
  class MidiOutputDevice {
  private:
    // DONE-rust
    int id_;
  public:
    // DONE-rust
    explicit MidiOutputDevice(int id);

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

    // TODO-rust
    void send(const helgoboss::MidiMessage& message, int frameOffset) const;

    // DONE-rust
    friend bool operator==(const MidiOutputDevice& lhs, const MidiOutputDevice& rhs);

    // DONE-rust
    friend bool operator!=(const MidiOutputDevice& lhs, const MidiOutputDevice& rhs);
  private:
    // TODO-rust
    inline midi_Output* load() const;
  };
}

