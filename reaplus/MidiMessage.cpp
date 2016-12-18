#include "MidiMessage.h"

namespace reaplus {
  int MidiMessage::frameOffset() const {
    return frameOffset_;
  }

  int MidiMessage::size() const {
    return (int) bytes_.size();
  }

  unsigned char MidiMessage::byte(int i) const {
    return bytes_.at((size_t) i);
  }

  MidiMessage::MidiMessage(const MIDI_event_t& event) : frameOffset_(event.frame_offset) {
    bytes_.reserve((size_t) event.size);
    for (int i = 0; i < event.size; i++) {
      bytes_.push_back(event.midi_message[i]);
    }
  }

  MidiMessageType MidiMessage::type() const {
    // Truncate low nibble from status byte
    const auto lowNibble = (unsigned char) (statusByte() & 0xf0);
    return static_cast<MidiMessageType>(lowNibble);
  }

  unsigned char MidiMessage::statusByte() const {
    return byte(0);
  }

  unsigned char MidiMessage::dataByte1() const {
    return byte(1);
  }

  unsigned char MidiMessage::dataByte2() const {
    return byte(2);
  }

  bool MidiMessage::isNoteOn() const {
    return type() == MidiMessageType::NoteOn && velocity() > 0;
  }

  bool MidiMessage::isNoteOff() const {
    return type() == MidiMessageType::NoteOff || type() == MidiMessageType::NoteOn && velocity() == 0;
  }

  bool MidiMessage::isNote() const {
    return type() == MidiMessageType::NoteOn || type() == MidiMessageType::NoteOff;
  }

  unsigned char MidiMessage::note() const {
    return dataByte1();
  }

  unsigned char MidiMessage::velocity() const {
    return dataByte2();
  }

  unsigned char MidiMessage::channel() const {
    // Extract low nibble from status byte
    return (unsigned char) (statusByte() & 0xf);
  }
}
