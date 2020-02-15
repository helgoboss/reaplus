#pragma once

#include <helgoboss-midi/MidiMessage.h>
#include "MidiInputDevice.h"

namespace reaplus {
  // DONE-rust
  class IncomingMidiEvent {
  private:
    MidiInputDevice inputDevice_;
    helgoboss::MidiMessage message_;
    int frameOffset_;
  public:
    IncomingMidiEvent(MidiInputDevice inputDevice, helgoboss::MidiMessage message, int frameOffset);
    MidiInputDevice inputDevice() const;
    helgoboss::MidiMessage message() const;
    int getFrameOffset() const;
  };
}

