#pragma once

#include "MidiMessage.h"
#include "MidiInputDevice.h"

namespace reaplus {
  class IncomingMidiEvent {
  private:
    MidiInputDevice inputDevice_;
    MidiMessage message_;
  public:
    IncomingMidiEvent(MidiInputDevice inputDevice, MidiMessage message);
    MidiInputDevice inputDevice() const;
    MidiMessage message() const;
  };
}

