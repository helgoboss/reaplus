#pragma once

#include <reaper_plugin.h>
#include <string>
#include "MidiMessage.h"

namespace reaplus {
  class MidiOutputDevice {
  private:
    int id_;
  public:
    MidiOutputDevice(int id);

    int id() const;

    std::string name() const;

    bool isAvailable() const;

    void send(const MidiMessage& message) const;

    friend bool operator==(const MidiOutputDevice& lhs, const MidiOutputDevice& rhs);

    friend bool operator!=(const MidiOutputDevice& lhs, const MidiOutputDevice& rhs);
  private:
    inline midi_Output* load() const;
  };
}

