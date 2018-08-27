#include <reaplus/RecordingInput.h>
#include <cmath>

namespace reaplus {
  int RecordingInput::recInputIndex() const {
    return recInputIndex_;
  }

  RecordingInput::RecordingInput(int recInputIndex) : recInputIndex_(recInputIndex) {

  }

  int MidiRecordingInput::midiRecInputIndex() const {
    return recInputIndex() - 4096;
  }

  boost::optional<MidiInputDevice> MidiRecordingInput::device() const {
    const auto rawDeviceId = (int) std::floor(midiRecInputIndex() / 32.0);
    return rawDeviceId == 63 ? boost::none : boost::make_optional(MidiInputDevice(rawDeviceId));
  }

  boost::optional<int> MidiRecordingInput::channel() const {
    const int channelId = midiRecInputIndex() % 32;
    return channelId == 0 ? boost::none : boost::make_optional(channelId - 1);
  }

  RecordingInputType MidiRecordingInput::type() const {
    return RecordingInputType::Midi;
  }

  MidiRecordingInput::MidiRecordingInput(int recInputIndex) : RecordingInput(recInputIndex) {
  }

  MidiRecordingInput MidiRecordingInput::fromAllDevicesAndChannels() {
    return MidiRecordingInput(4096 + 63 * 32);
  }

  MidiRecordingInput MidiRecordingInput::fromAllChannelsOfDevice(int deviceId) {
    return MidiRecordingInput(4096 + deviceId * 32);
  }

  MidiRecordingInput MidiRecordingInput::ofMidiRecInputIndex(int midiRecInputIndex) {
    return MidiRecordingInput(4096 + midiRecInputIndex);
  }

  MidiRecordingInput MidiRecordingInput::fromAllDevicesWithChannel(int channel) {
    return fromDeviceAndChannel(63, channel);
  }

  MidiRecordingInput MidiRecordingInput::fromDeviceAndChannel(int deviceId, int channel) {
    return MidiRecordingInput(4096 + deviceId * 32 + channel + 1);
  }

  std::unique_ptr<RecordingInput> RecordingInput::ofRecInputIndex(int recInputIndex) {
    if (recInputIndex >= 4096) {
      return std::unique_ptr<MidiRecordingInput>(new MidiRecordingInput(recInputIndex));
    } else {
      return nullptr;
    }
  }

  bool reaplus::operator==(const RecordingInput& lhs, const RecordingInput& rhs) {
    return lhs.recInputIndex_ == rhs.recInputIndex_;
  }
}