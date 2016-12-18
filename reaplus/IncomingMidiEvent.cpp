#include "IncomingMidiEvent.h"

namespace reaplus {
  MidiInputDevice IncomingMidiEvent::inputDevice() const {
    return inputDevice_;
  }

  MidiMessage IncomingMidiEvent::message() const {
    return message_;
  }

  IncomingMidiEvent::IncomingMidiEvent(MidiInputDevice inputDevice, MidiMessage message) : inputDevice_(inputDevice),
    message_(message){

  }
}