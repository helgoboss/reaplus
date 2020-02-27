#pragma once

#include <memory>
#include <boost/optional.hpp>
#include "MidiInputDevice.h"

namespace reaplus {
  // TODO Also support record inputs other than MIDI
  // DONE-rust
  enum class RecordingInputType {
    Midi
  };

  // DONE-rust
  class RecordingInput {
  private:
    const int recInputIndex_;
  public:
    // DONE-rust
    static std::unique_ptr<RecordingInput> ofRecInputIndex(int recInputIndex);

    // DONE-rust
    int recInputIndex() const;
    // DONE-rust
    virtual RecordingInputType type() const = 0;
    // DONE-rust
    friend bool operator==(const RecordingInput& lhs, const RecordingInput& rhs);

  protected:
    // DONE-rust
    explicit RecordingInput(int recInputIndex);
  };

  // DONE-rust
  class MidiRecordingInput : public RecordingInput {
    friend class RecordingInput;
  public:
    // TODO Rename 'from' to 'of'
    // DONE-rust
    static MidiRecordingInput fromAllDevicesAndChannels();
    // DONE-rust
    static MidiRecordingInput fromAllChannelsOfDevice(int deviceId);
    // DONE-rust
    static MidiRecordingInput fromAllDevicesWithChannel(int channel);
    // DONE-rust
    static MidiRecordingInput fromDeviceAndChannel(int deviceId, int channel);
    // DONE-rust
    static MidiRecordingInput ofMidiRecInputIndex(int midiRecInputIndex);
    // DONE-rust
    int midiRecInputIndex() const;
    // DONE-rust
    boost::optional<MidiInputDevice> device() const;
    // DONE-rust
    boost::optional<int> channel() const;

    // DONE-rust
    RecordingInputType type() const override;

  protected:
    // DONE-rust
    explicit MidiRecordingInput(int recInputIndex);
  };
}