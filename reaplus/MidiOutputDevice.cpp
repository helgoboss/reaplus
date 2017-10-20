#include "MidiOutputDevice.h"
#include <reaplus/utility.h>
#include <reaper_plugin_functions.h>


namespace reaplus {
  MidiOutputDevice::MidiOutputDevice(int id) : id_(id), midiOutput_(nullptr) {
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
  
  void MidiOutputDevice::send(const MidiMessage &message) const {
    loadIfNecessaryOrComplain();
    midiOutput_->Send(message.statusByte(), message.dataByte1(), message.dataByte2(), message.frameOffset());
  }

  bool reaplus::operator==(const MidiOutputDevice& lhs, const MidiOutputDevice& rhs) {
    return lhs.id_ == rhs.id_;
  }

  bool reaplus::operator!=(const MidiOutputDevice& lhs, const MidiOutputDevice& rhs) {
    return !(lhs == rhs);
  }

  void MidiOutputDevice::loadIfNecessaryOrComplain() const {
    if (midiOutput_ == nullptr && !loadById()) {
      throw std::logic_error("Device not loadable");
    }
  }

  bool MidiOutputDevice::loadById() const {
    midiOutput_ = reaper::GetMidiOutput(id_);
    return midiOutput_ != nullptr;
  }
}