#pragma once

#include <reaper_plugin.h>
#include <string>
#include "MidiMessage.h"

namespace reaplus {
  class MidiOutputDevice {
  private:
    int id_;
  public:
    explicit MidiOutputDevice(int id);

    int id() const;

    std::string name() const;

    // For REAPER < 5.94 this is the same like isConnected(). For REAPER >=5.94 it returns true if the device ever
    // existed, even if it's disconnected now.
    bool isAvailable() const;

    // Only returns true if the device is connected (= present)
    bool isConnected() const;

    void send(const MidiMessage& message) const;

    friend bool operator==(const MidiOutputDevice& lhs, const MidiOutputDevice& rhs);

    friend bool operator!=(const MidiOutputDevice& lhs, const MidiOutputDevice& rhs);
  private:
    inline midi_Output* load() const;
  };
}

