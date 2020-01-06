#include <reaplus/IncomingMidiEvent.h>
#include <utility>

using helgoboss::MidiMessage;

namespace reaplus {
  MidiInputDevice IncomingMidiEvent::inputDevice() const {
    return inputDevice_;
  }

  MidiMessage IncomingMidiEvent::message() const {
    return message_;
  }
  
  int IncomingMidiEvent::getFrameOffset() const {
    return frameOffset_;
  }

  IncomingMidiEvent::IncomingMidiEvent(MidiInputDevice inputDevice, MidiMessage message, int frameOffset)
      : inputDevice_(inputDevice), message_(std::move(message)), frameOffset_(frameOffset) {
  }
}