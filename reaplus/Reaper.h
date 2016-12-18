#pragma once

#include <string>
#include <functional>
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
  class IncomingMidiEvent;

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


    audio_hook_register_t audioHook_;
    std::unordered_map<int, Command> commandByIndex_;
    rxcpp::subjects::subject<IncomingMidiEvent> incomingMidiEventsSubject_;

  public:
    static Reaper& instance();

    Reaper();

    ~Reaper();

    RegisteredAction registerAction(const std::string& commandId, const std::string& description,
        std::function<void()> operation, std::function<bool()> isOn = nullptr);

    boost::optional<FxParameter> lastTouchedFxParameter() const;

    Project currentProject() const;

    Project createNewDefaultProjectInCurrentTab();

    Project createNewDefaultProjectInNewTab();

    Project createEmptyProjectInNewTab();

    rxcpp::subscription executeInMainThread(std::function<void(void)> command);

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

    rxcpp::observable<MidiInputDevice> midiInputDevices() const;

    // It's correct that this method returns a non-optional. An id is supposed to uniquely identify a device.
    // A MidiInputDevice#isAvailable method returns if the device is actually existing at runtime. That way we
    // support (still) unloaded MidiInputDevices.
    MidiInputDevice midiInputDeviceById(int id) const;

    AutomationMode globalAutomationOverride() const;

    Section mainSection() const;

    // It's correct that this method returns a non-optional. A commandName is supposed to uniquely identify the action,
    // so it could be part of the resulting Action itself. An Action#isAvailable method could return if the action is
    // actually existing at runtime. That way we would support (still) unloaded Actions.
    // TODO Don't automatically interpret command name as commandId
    Action actionByCommandName(std::string commandName) const;

  private:
    static bool staticHookCommand(int commandIndex, int flag);
    static int staticToggleAction(int commandIndex);
    static void processAudioBuffer(bool isPost, int len, double srate, struct audio_hook_register_t *reg);
  };
}

