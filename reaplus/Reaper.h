#pragma once

#include <string>
#include <functional>
#include <thread>
#include <memory>
#include <unordered_map>
#include <reaper_plugin.h>
#include <rxcpp/rx.hpp>
#include <boost/optional.hpp>
#include "RegisteredAction.h"
#include "AutomationMode.h"
#include "Guid.h"

namespace reaplus {
  class Action;
  class FxParameter;
  class Project;
  class Section;
  class MidiInputDevice;
  class MidiOutputDevice;
  class IncomingMidiEvent;
  class Parameter;
  class TrackSend;
  class Fx;
  class Track;

  enum class MessageBoxType : int {
    Ok,
    OkCancel,
    AbortRetryIgnore,
    YesNoCancel,
    YesNo,
    RetryCancel
  };

  enum class MessageBoxResult : int {
    Ok = 1,
    Cancel,
    Abort,
    Retry,
    Ignore,
    Yes,
    No
  };

  class Reaper {
    friend class RegisteredAction;

  private:
    class Command {
    private:
      std::string description_;
      std::function<void()> operation_;
      std::function<bool()> isOn_;
      gaccel_register_t acceleratorRegister_;
    public:
      Command(int commandIndex, std::string description, std::function<void()> operation, std::function<bool()> isOn = nullptr);

      // Don't know for sure but might be that REAPER doesn't copy the acceleratorRegister_ on registration. So copying (in particular after registration) is forbidden.
      Command(const Command& that) = delete;

      Command(const Command&& that);

      void registerIt();

      void unregister();

      void execute();

      bool reportsOnOffState() const;

      bool isOn() const;
    };


    std::thread::id idOfMainThread_;
    audio_hook_register_t audioHook_;
    std::unordered_map<int, Command> commandByIndex_;
    rxcpp::subjects::subject<IncomingMidiEvent> incomingMidiEventsSubject_;
    rxcpp::subjects::subject<Action> actionInvokedSubject_;

  public:
    static Reaper& instance();

    void init();

    Reaper();

    ~Reaper();

    RegisteredAction registerAction(const std::string& commandId, const std::string& description,
        std::function<void()> operation, std::function<bool()> isOn = nullptr);

    boost::optional<FxParameter> lastTouchedFxParameter() const;

    Project currentProject() const;

    Project createNewDefaultProjectInCurrentTab();

    Project createNewDefaultProjectInNewTab();

    Project createEmptyProjectInNewTab();

    void showConsoleMessage(const std::string& msg);

    // type 0=OK,1=OKCANCEL,2=ABORTRETRYIGNORE,3=YESNOCANCEL,4=YESNO,5=RETRYCANCEL : ret 1=OK,2=CANCEL,3=ABORT,4=RETRY,5=IGNORE,6=YES,7=NO

    MessageBoxResult showMessageBox(const std::string& msg, const std::string& title, MessageBoxType type);

    void clearConsole();

    std::string exePath() const;

    int projectCount() const;

    Guid generateGuid() const;

    rxcpp::observable<IncomingMidiEvent> incomingMidiEvents() const;

    // It's correct that this method returns an optional because the index isn't a stable identifier of a project.
    // The project could move. So this should do a runtime lookup of the project and return a stable ReaProject-backed
    // Project object if a project exists at that index.
    boost::optional<Project> projectByIndex(int index) const;

    rxcpp::observable<Project> projects() const;

    rxcpp::observable<Project> projectsWithCurrentOneFirst() const;

    rxcpp::observable<MidiInputDevice> midiInputDevices() const;

    rxcpp::observable<MidiOutputDevice> midiOutputDevices() const;

    // It's correct that this method returns a non-optional. An id is supposed to uniquely identify a device.
    // A MidiInputDevice#isAvailable method returns if the device is actually existing at runtime. That way we
    // support (still) unloaded MidiInputDevices.
    MidiInputDevice midiInputDeviceById(int id) const;

    // It's correct that this method returns a non-optional. An id is supposed to uniquely identify a device.
    // A MidiOutputDevice#isAvailable method returns if the device is actually existing at runtime. That way we
    // support (still) unloaded MidiOutputDevices.
    MidiOutputDevice midiOutputDeviceById(int id) const;

    AutomationMode globalAutomationOverride() const;

    Section mainSection() const;

    // It's correct that this method returns a non-optional. A commandName is supposed to uniquely identify the action,
    // so it could be part of the resulting Action itself. An Action#isAvailable method could return if the action is
    // actually existing at runtime. That way we would support (still) unloaded Actions.
    // TODO Don't automatically interpret command name as commandId
    Action actionByCommandName(std::string commandName) const;


    rxcpp::observable<Parameter*> parameterValueChangedUnsafe() const;

    rxcpp::observable<Parameter*> parameterTouchedUnsafe() const;

    rxcpp::observable<FxParameter> fxParameterValueChanged() const;

    rxcpp::observable<FxParameter> fxParameterTouched() const;

    rxcpp::observable<Track> trackVolumeChanged() const;

    rxcpp::observable<Track> trackVolumeTouched() const;

    rxcpp::observable<Track> trackPanChanged() const;

    rxcpp::observable<Track> trackNameChanged() const;

    rxcpp::observable<Track> trackInputChanged() const;

    rxcpp::observable<Track> trackPanTouched() const;

    rxcpp::observable<Track> trackArmTouched() const;

    rxcpp::observable<TrackSend> trackSendVolumeChanged() const;

    rxcpp::observable<TrackSend> trackSendVolumeTouched() const;

    rxcpp::observable<Track> trackAdded() const;

    // Delivers a GUID-based track (to still be able to identify it even it is deleted)
    rxcpp::observable<Track> trackRemoved() const;

    rxcpp::observable<Project> tracksReordered() const;

    rxcpp::observable<Fx> fxAdded() const;

    rxcpp::observable<Fx> fxRemoved() const;

    rxcpp::observable<Fx> fxEnabledChanged() const;

    rxcpp::observable<Track> fxReordered() const;

    rxcpp::observable<Track> trackInputMonitoringChanged() const;

    rxcpp::observable<Track> trackArmChanged() const;

    rxcpp::observable<Track> trackMuteChanged() const;

    rxcpp::observable<Track> trackSoloChanged() const;

    rxcpp::observable<Track> trackSelectedChanged() const;

    rxcpp::observable<Project> projectSwitched() const;

    rxcpp::observable<Action> actionInvoked() const;

    rxcpp::subscription executeLaterInMainThread(std::function<void(void)> command);

    rxcpp::subscription executeWhenInMainThread(std::function<void(void)> command);

    const rxcpp::observe_on_one_worker& mainThreadCoordination() const;

    // Attention: Returns normal fx only, not input fx!
    // This is not reliable! After REAPER start no focused Fx can be found!
    boost::optional<Fx> focusedFx() const;

    std::thread::id idOfMainThread() const;

  private:
    // Only for main section
    static void staticHookPostCommand(int commandId, int flag);
    // Only for main section
    static bool staticHookCommand(int commandIndex, int flag);
    // Only for main section
    static int staticToggleAction(int commandIndex);
    static void processAudioBuffer(bool isPost, int len, double srate, struct audio_hook_register_t *reg);
  };
}

