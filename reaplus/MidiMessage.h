#pragma once

#include <vector>
#include <reaper_plugin.h>

namespace reaplus {
  enum class MidiMessageType : unsigned char {
    NoteOn = 0x90,
    NoteOff = 0x80,
    Cc = 0xb0,
    PitchWheel = 0xb0,
    ProgramChange = 0xc0,
    ChannelAftertouch = 0xd0,
    PolyAftertouch = 0xa0,
    ActiveSensing = 0xfe
  };
  class MidiMessage {
    friend class Reaper;
  private:
    int frameOffset_;
    std::vector<unsigned char> bytes_;
  public:
    static MidiMessage noteOn(int channel, int noteNumber, int velocity, int frameOffset);
    static MidiMessage noteOff(int channel, int noteNumber, int velocity, int frameOffset);
    static MidiMessage cc(int channel, int ccNumber, int ccValue, int frameOffset);
    static MidiMessage pitchWheel(int channel, int pitchValue, int frameOffset);
    static MidiMessage channelAftertouch(int channel, int aftertouchValue, int frameOffset);
    static MidiMessage programChange(int channel, int programIndex, int frameOffset);
    static inline MidiMessage simple(MidiMessageType type, int channel, int data1, int data2, int frameOffset);
//    static MidiMessage rpn(int channel, int rpnNumber, int rpnValue, int frameOffset);
//    static MidiMessage nrpn(int channel, int nrpnNumber, int nrpnValue, int frameOffset);
    static std::pair<MidiMessage, MidiMessage> fourteenBitCc(int channel, int ccNumber, int ccValue, int frameOffset);

    MidiMessage(unsigned char statusByte, unsigned char dataByte1, unsigned char dataByte2, int frameOffset);
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

