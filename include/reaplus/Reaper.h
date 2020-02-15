#pragma once

#include <string>
#include <functional>
#include <boost/filesystem.hpp>
#include <thread>
#include <memory>
#include <unordered_map>
#include <reaper_plugin.h>
#include <rxcpp/rx.hpp>
#include <boost/optional.hpp>
#include "RegisteredAction.h"
#include "AutomationMode.h"
#include <helgoboss-midi/MidiMessage.h>
#include "Guid.h"
#include "util/rx-relaxed-runloop.hpp"

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

  // DONE-rust
  enum class MessageBoxType : int {
    Ok,
    OkCancel,
    AbortRetryIgnore,
    YesNoCancel,
    YesNo,
    RetryCancel
  };

  // DONE-rust
  enum class MessageBoxResult : int {
    Ok = 1,
    Cancel,
    Abort,
    Retry,
    Ignore,
    Yes,
    No
  };

  // DONE-rust
  enum class StuffMidiMessageTarget {
    VirtualMidiKeyboard,
    MidiAsControlInputQueue,
    VirtualMidiKeyboardOnCurrentChannel
  };

  // TODO-rust
  struct ProjectConfigExtension {
    std::function<bool(const char*, ProjectStateContext*, bool, struct project_config_extension_t*)>
        processExtensionLine;
    std::function<void(bool, struct project_config_extension_t*)> beginLoadProjectState;
    std::function<void(ProjectStateContext*, bool, struct project_config_extension_t*)> saveExtensionConfig;
  };

  class Reaper {
    friend class RegisteredAction;

  private:
    // TODO-rust
    class Guard {
    public:
      ~Guard();
    };

    class Command {
    private:
      // DONE-rust
      std::string description_;
      std::function<void()> operation_;
      std::function<bool()> isOn_;
      gaccel_register_t acceleratorRegister_;
    public:
      // DONE-rust
      Command(int commandIndex,
          std::string description,
          std::function<void()> operation,
          std::function<bool()> isOn = nullptr);

      // DONE-rust
      // Don't know for sure but might be that REAPER doesn't copy the acceleratorRegister_ on registration. So copying (in particular after registration) is forbidden.
      Command(const Command& that) = delete;

      // DONE-rust
      Command(Command&& that) noexcept;

      // DONE-rust
      void registerIt();

      // DONE-rust
      void unregister();

      // DONE-rust
      void execute();

      // DONE-rust
      bool reportsOnOffState() const;

      // DONE-rust
      bool isOn() const;
    };

    // DONE-rust
    static std::unique_ptr<Reaper> INSTANCE;
    // DONE-rust
    std::thread::id idOfMainThread_;
    // DONE-rust
    audio_hook_register_t audioHook_;
    // TODO-rust
    project_config_extension_t projectConfigExtension_;
    // DONE-rust
    std::unordered_map<int, Command> commandByIndex_;
    // TODO-rust
    std::vector<ProjectConfigExtension> projectConfigExtensions_;
    // DONE-rust
    rxcpp::subjects::subject<IncomingMidiEvent> incomingMidiEventsSubject_;
    // DONE-rust
    rxcpp::subjects::subject<Action> actionInvokedSubject_;
    // TODO-rust
    uint64_t sampleCounter_ = 0;
    rxcpp::schedulers::relaxed_run_loop audioThreadRunLoop_;
    rxcpp::observe_on_one_worker audioThreadCoordination_ =
        rxcpp::observe_on_one_worker(rxcpp::schedulers::make_relaxed_run_loop(audioThreadRunLoop_));

  public:
    // DONE-rust
    static Reaper& instance();

    /**
     * Destroys the single instance of this class. Also takes care of destroying the HelperControlSurfaces. Most
     * importantly, this triggers a lot of unregistrations from REAPER (hooks, control surface etc.).
     * In case you use ReaPlus from a VST plug-in (which unlike REAPER extensions most likely have a smaller lifetime
     * than REAPER itself), you *must* call this function as soon as the last instance of your VST plug-in is destroyed.
     * But you should do it in extensions as well.
     */
     // TODO-rust
    static void destroyInstance();

    // DONE-rust
    ~Reaper();

    // DONE-rust
    void init();

    // DONE-rust
    RegisteredAction registerAction(const std::string& commandId, const std::string& description,
        std::function<void()> operation, std::function<bool()> isOn = nullptr);

    // TODO-rust
    // TODO Make it unregisterable
    // TODO Make it more intuitive (like registering actions)
    void registerProjectConfigExtension(ProjectConfigExtension extension);

    // TODO-rust
    boost::optional<FxParameter> lastTouchedFxParameter() const;

    // DONE-rust
    Project currentProject() const;

    // TODO-rust
    Project createNewDefaultProjectInCurrentTab();

    // TODO-rust
    Project createNewDefaultProjectInNewTab();

    // DONE-rust
    Project createEmptyProjectInNewTab();

    // DONE-rust
    void showConsoleMessage(const std::string& msg);

    // type 0=OK,1=OKCANCEL,2=ABORTRETRYIGNORE,3=YESNOCANCEL,4=YESNO,5=RETRYCANCEL : ret 1=OK,2=CANCEL,3=ABORT,4=RETRY,5=IGNORE,6=YES,7=NO
    // DONE-rust
    MessageBoxResult showMessageBox(const std::string& msg, const std::string& title, MessageBoxType type);

    // DONE-rust
    void clearConsole();

    // TODO-rust
    std::string exePath() const;

    // TODO-rust
    boost::filesystem::path getResourceDir() const;

    // DONE-rust
    int projectCount() const;

    // DONE-rust
    Guid generateGuid() const;

    // DONE-rust
    rxcpp::observable<IncomingMidiEvent> incomingMidiEvents() const;

    // It's correct that this method returns an optional because the index isn't a stable identifier of a project.
    // The project could move. So this should do a runtime lookup of the project and return a stable ReaProject-backed
    // Project object if a project exists at that index.
    boost::optional<Project> projectByIndex(int index) const;

    // DONE-rust
    rxcpp::observable<Project> projects() const;

    // TODO-rust Maybe unnecessary
    rxcpp::observable<Project> projectsWithCurrentOneFirst() const;

    // DONE-rust
    rxcpp::observable<MidiInputDevice> midiInputDevices() const;

    // DONE-rust
    rxcpp::observable<MidiOutputDevice> midiOutputDevices() const;

    // It's correct that this method returns a non-optional. An id is supposed to uniquely identify a device.
    // A MidiInputDevice#isAvailable method returns if the device is actually existing at runtime. That way we
    // support (still) unloaded MidiInputDevices.
    // DONE-rust
    MidiInputDevice midiInputDeviceById(int id) const;

    // It's correct that this method returns a non-optional. An id is supposed to uniquely identify a device.
    // A MidiOutputDevice#isAvailable method returns if the device is actually existing at runtime. That way we
    // support (still) unloaded MidiOutputDevices.
    // DONE-rust
    MidiOutputDevice midiOutputDeviceById(int id) const;

    // DONE-rust
    AutomationMode globalAutomationOverride() const;

    // DONE-rust
    Section mainSection() const;

    // It's correct that this method returns a non-optional. A commandName is supposed to uniquely identify the action,
    // so it could be part of the resulting Action itself. An Action#isAvailable method could return if the action is
    // actually existing at runtime. That way we would support (still) unloaded Actions.
    // TODO Don't automatically interpret command name as commandId
    // DONE-rust
    Action actionByCommandName(std::string commandName) const;

    rxcpp::observable<Parameter*> parameterValueChangedUnsafe() const;

    rxcpp::observable<Parameter*> parameterTouchedUnsafe() const;

    rxcpp::observable<FxParameter> fxParameterValueChanged() const;

    rxcpp::observable<FxParameter> fxParameterTouched() const;

    // TODO-rust
    rxcpp::observable<Fx> fxOpened() const;

    // TODO-rust
    rxcpp::observable<Fx> fxClosed() const;

    // TODO-rust
    rxcpp::observable<boost::optional<Fx>> fxFocused() const;

    // DONE-rust
    rxcpp::observable<Track> trackVolumeChanged() const;

    // TODO-rust
    rxcpp::observable<Track> trackVolumeTouched() const;

    // DONE-rust
    rxcpp::observable<Track> trackPanChanged() const;

    // DONE-rust
    rxcpp::observable<Track> trackNameChanged() const;

    // DONE-rust
    rxcpp::observable<Track> trackInputChanged() const;

    // TODO-rust
    rxcpp::observable<Track> trackPanTouched() const;

    // TODO-rust
    rxcpp::observable<Track> trackArmTouched() const;

    // DONE-rust
    rxcpp::observable<TrackSend> trackSendVolumeChanged() const;

    // TODO-rust
    rxcpp::observable<TrackSend> trackSendVolumeTouched() const;

    // DONE-rust
    rxcpp::observable<TrackSend> trackSendPanChanged() const;

    // TODO-rust
    rxcpp::observable<TrackSend> trackSendPanTouched() const;

    // DONE-rust
    rxcpp::observable<Track> trackAdded() const;

    // Delivers a GUID-based track (to still be able to identify it even it is deleted)
    rxcpp::observable<Track> trackRemoved() const;

    // TODO-rust
    rxcpp::observable<Project> tracksReordered() const;

    // DONE-rust
    rxcpp::observable<Fx> fxAdded() const;

    // TODO-rust
    rxcpp::observable<Fx> fxRemoved() const;

    // DONE-rust
    rxcpp::observable<Fx> fxEnabledChanged() const;

    // TODO-rust
    rxcpp::observable<Fx> fxEnabledTouched() const;

    // TODO-rust
    rxcpp::observable<Track> fxReordered() const;

    // DONE-rust
    rxcpp::observable<Track> trackInputMonitoringChanged() const;

    // DONE-rust
    rxcpp::observable<Track> trackArmChanged() const;

    // DONE-rust
    rxcpp::observable<Track> trackMuteChanged() const;

    // TODO-rust
    rxcpp::observable<Track> trackMuteTouched() const;

    // DONE-rust
    rxcpp::observable<Track> trackSoloChanged() const;

    // TODO-rust
    rxcpp::observable<Track> trackSoloTouched() const;

    // DONE-rust
    rxcpp::observable<Track> trackSelectedChanged() const;

    // TODO-rust
    rxcpp::observable<Track> trackSelectedTouched() const;

    // DONE-rust
    rxcpp::observable<Project> projectSwitched() const;

    // TODO-rust
    rxcpp::observable<Project> projectClosed() const;

    // DONE-rust
    rxcpp::observable<Action> actionInvoked() const;

    // DONE-rust
    rxcpp::observable<bool> masterTempoChanged() const;

    // TODO-rust
    rxcpp::observable<bool> masterTempoTouched() const;

    // TODO-rust
    rxcpp::observable<bool> masterPlayrateChanged() const;

    // TODO-rust
    rxcpp::observable<bool> masterPlayrateTouched() const;

    // TODO-rust
    rxcpp::observable<bool> mainThreadIdle() const;

    rxcpp::composite_subscription executeLaterInMainThread(std::function<void(void)> command);

    // DONE-rust
    void executeLaterInMainThreadFast(std::function<void(void)> command);

    // DONE-rust (without subscription)
    rxcpp::composite_subscription executeWhenInMainThread(std::function<void(void)> command);

    const rxcpp::observe_on_one_worker& mainThreadCoordination() const;

    const rxcpp::observe_on_one_worker& audioThreadCoordination() const;

    // Attention: Returns normal fx only, not input fx!
    // This is not reliable! After REAPER start no focused Fx can be found!
    // TODO-rust
    boost::optional<Fx> focusedFx() const;

    // DONE-rust
    std::thread::id idOfMainThread() const;

    // DONE-rust
    bool currentThreadIsMainThread() const;

    // TODO-rust
    boost::optional<Project> getCurrentlyRenderingProject() const;

    // DONE-rust
    HWND mainWindow() const;

    // TODO-rust
    uint64_t sampleCounter() const;

    // DONE-rust
    void stuffMidiMessage(StuffMidiMessageTarget target, const helgoboss::MidiMessage& message);

    // DONE-rust
    std::string getVersion() const;

  private:
    // DONE-rust
    Reaper();
    Reaper(const Reaper&);

    // Only for main section
    static void staticHookPostCommand(int commandId, int flag);
    // Only for main section
    // DONE-rust
    static bool staticHookCommand(int commandIndex, int flag);
    // Only for main section
    static int staticToggleAction(int commandIndex);

    // DONE-rust
    static void processAudioBuffer(bool isPost, int len, double srate, struct audio_hook_register_t* reg);
    static bool processExtensionLine(const char* line, ProjectStateContext* ctx, bool isUndo,
        struct project_config_extension_t* reg);
    static void beginLoadProjectState(bool isUndo, struct project_config_extension_t* reg);
    static void saveExtensionConfig(ProjectStateContext* ctx, bool isUndo, struct project_config_extension_t* reg);
    
    static helgoboss::MidiMessage createMidiMessageFromEvent(const MIDI_event_t& event);
  };
}

