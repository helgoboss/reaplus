#pragma once

#include <memory>
#include <boost/optional.hpp>
#include "MidiInputDevice.h"

namespace reaplus {
  // TODO Also support record inputs other than MIDI
  enum class RecordingInputType {
    Midi
  };

  class RecordingInput {
  private:
    const int recInputIndex_;
  public:
    static std::unique_ptr<RecordingInput> ofRecInputIndex(int recInputIndex);

    int recInputIndex() const;
    virtual RecordingInputType type() const = 0;
    friend bool operator==(const RecordingInput& lhs, const RecordingInput& rhs);

  protected:
    explicit RecordingInput(int recInputIndex);
  };

  class MidiRecordingInput : public RecordingInput {
    friend class RecordingInput;
  public:
    // TODO Rename 'from' to 'of'
    static MidiRecordingInput fromAllDevicesAndChannels();
    static MidiRecordingInput fromAllChannelsOfDevice(int deviceId);
    static MidiRecordingInput fromAllDevicesWithChannel(int channel);
    static MidiRecordingInput fromDeviceAndChannel(int deviceId, int channel);
    static MidiRecordingInput ofMidiRecInputIndex(int midiRecInputIndex);
    int midiRecInputIndex() const;
    boost::optional<MidiInputDevice> device() const;
    boost::optional<int> channel() const;

    RecordingInputType type() const override;

  protected:
    explicit MidiRecordingInput(int recInputIndex);
  };
}