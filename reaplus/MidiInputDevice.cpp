#include "MidiInputDevice.h"
#include <reaplus/utility.h>
#include <reaper_plugin_functions.h>

namespace reaplus {
  MidiInputDevice::MidiInputDevice(int id) : id_(id) {
  }

  int MidiInputDevice::id() const {
    return id_;
  }

  std::string MidiInputDevice::name() const {
    return reaplus::toString(33, [this](char* buffer, int maxSize) {
      reaper::GetMIDIInputName(id_, buffer, maxSize);
    });
  }

  bool MidiInputDevice::isAvailable() const {
    char name[2] = {0};
    const bool connected = reaper::GetMIDIInputName(id_, name, 2);
    return connected || name[0] != 0;
  }

  bool MidiInputDevice::isConnected() const {
    // In REAPER 5.94 GetMIDIInputName doesn't accept nullptr as name buffer on OS X
    char dummy[1];
    return reaper::GetMIDIInputName(id_, dummy, 0);
  }

  bool reaplus::operator==(const MidiInputDevice& lhs, const MidiInputDevice& rhs) {
    return lhs.id_ == rhs.id_;
  }

  bool reaplus::operator!=(const MidiInputDevice& lhs, const MidiInputDevice& rhs) {
    return !(lhs == rhs);
  }
}