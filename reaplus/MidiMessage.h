#pragma once

#include <vector>
#include <reaper_plugin.h>

namespace reaplus {
  enum class MidiMessageType : unsigned char {
    NoteOn = 0x90,
    NoteOff = 0x80,
    Cc = 0xb0,
    ActiveSensing = 254
  };
  class MidiMessage {
    friend class Reaper;
  private:
    int frameOffset_;
    std::vector<unsigned char> bytes_;
  public:
    int frameOffset() const;
    int size() const;
    unsigned char byte(int i) const;
    unsigned char statusByte() const;
    unsigned char dataByte1() const;
    unsigned char dataByte2() const;
    MidiMessageType type() const;
    bool isNoteOn() const;
    bool isNoteOff() const;
    bool isNote() const;
    unsigned char note() const;
    unsigned char velocity() const;
    unsigned char channel() const;
  private:
    MidiMessage(const MIDI_event_t& event);
  };
}

