#include "MidiOutputDevice.h"
#include <reaplus/utility.h>
#include <reaper_plugin_functions.h>

namespace reaplus {
  MidiOutputDevice::MidiOutputDevice(int id) : id_(id) {
  }

  int MidiOutputDevice::id() const {
    return id_;
  }

  std::string MidiOutputDevice::name() const {
    return reaplus::toString(33, [this](char* buffer, int maxSize) {
      reaper::GetMIDIOutputName(id_, buffer, maxSize);
    });
  }

  bool MidiOutputDevice::isAvailable() const {
    return reaper::GetMIDIOutputName(id_, nullptr, 0);
  }

  void MidiOutputDevice::send(const MidiMessage& message) const {
    if (auto midiOutput = load()) {
      midiOutput->Send(message.statusByte(), message.dataByte1(), message.dataByte2(), message.frameOffset());
    }
  }

  bool reaplus::operator==(const MidiOutputDevice& lhs, const MidiOutputDevice& rhs) {
    return lhs.id_ == rhs.id_;
  }

  bool reaplus::operator!=(const MidiOutputDevice& lhs, const MidiOutputDevice& rhs) {
    return !(lhs == rhs);
  }

  midi_Output* MidiOutputDevice::load() const {
    return reaper::GetMidiOutput(id_);
  }
}