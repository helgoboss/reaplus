#include <reaplus/HelperControlSurface.h>
#include <reaper_plugin.h>
#include <dbdaoint.h>
#include "Reaper.h"
#include "FxParameter.h"
#include "Project.h"
#include "Section.h"
#include "Action.h"
#include "MidiInputDevice.h"
#include "IncomingMidiEvent.h"
#include "HelperControlSurface.h"

#include <reaper_plugin_functions.h>

using rxcpp::subscriber;
using boost::none;
using std::string;
using std::string;
using std::function;
using std::unordered_map;
using std::unique_ptr;
using rxcpp::observable;
using boost::optional;


namespace reaplus {

  Reaper& reaplus::Reaper::instance() {
    static Reaper INSTANCE;
    return INSTANCE;
  }

  RegisteredAction Reaper::registerAction(const string& commandId, const string& description,
      function<void()> operation, function<bool()> isOn) {
    const int commandIndex = reaper::plugin_register("command_id", (void*) commandId.c_str());
    auto pair = commandByIndex_.emplace(commandIndex, Command(commandIndex, description, operation, isOn));
    Command& command = pair.first->second;
    command.registerIt();
    return RegisteredAction(commandIndex);
  }

  bool Reaper::staticHookCommand(int commandIndex, int flag) {
    auto& commandByIndex = Reaper::instance().commandByIndex_;
    if (commandByIndex.count(commandIndex)) {
      auto& command = commandByIndex.at(commandIndex);
      command.execute();
      return true;
    } else {
      return false;
    }
  }

  bool Reaper::Command::reportsOnOffState() const {
    return isOn_ != nullptr;
  }

  bool Reaper::Command::isOn() const {
    return isOn_();
  }

  int Reaper::staticToggleAction(int commandIndex) {
    auto& commandByIndex = Reaper::instance().commandByIndex_;
    if (commandByIndex.count(commandIndex)) {
      auto& command = commandByIndex.at(commandIndex);
      if (command.reportsOnOffState()) {
        return command.isOn() ? 1 : 0;
      } else {
        return -1;
      }
    } else {
      return -1;
    }
  }

  Reaper::Reaper() {
    reaper::plugin_register("hookcommand", (void*) &staticHookCommand);
    reaper::plugin_register("toggleaction", (void*) &staticToggleAction);
    audioHook_.OnAudioBuffer = &processAudioBuffer;
    reaper::Audio_RegHardwareHook(true, &audioHook_);
  }

  Reaper::~Reaper() {
    reaper::plugin_register("-toggleaction", (void*) &staticToggleAction);
    reaper::plugin_register("-hookcommand", (void*) &staticHookCommand);
  }

  optional<FxParameter> Reaper::lastTouchedFxParameter() const {
    // TODO Sucks: We have to assume it was a parameter in the current project
    // Maybe we should rather rely on our own technique in ControlSurface here!
    int trackIndex;
    int fxQueryIndex;
    int paramIndex;
    bool isValid = reaper::GetLastTouchedFX(&trackIndex, &fxQueryIndex, &paramIndex);
    if (isValid) {
      int normalizedTrackIndex = trackIndex - 1;
      if (normalizedTrackIndex >= Reaper::instance().currentProject().trackCount()) {
        // Must be in another project
        return none;
      } else {
        // Track exists in this project
        const auto track = normalizedTrackIndex == -1
                     ? Reaper::instance().currentProject().masterTrack()
                     : *Reaper::instance().currentProject().trackByIndex(normalizedTrackIndex);
        if (const auto fx = track.fxByQueryIndex(fxQueryIndex)) {
          return fx->parameterByIndex(paramIndex);
        } else {
          return none;
        }
      }
    } else {
      return none;
    }
  }

  Project Reaper::createNewDefaultProjectInCurrentTab() {
    mainSection().actionByCommandId(40023).invoke();
    return currentProject();
  }

  Project Reaper::createNewDefaultProjectInNewTab() {
    mainSection().actionByCommandId(40859).invoke();
    return currentProject();
  }

  Project Reaper::createEmptyProjectInNewTab() {
    mainSection().actionByCommandId(41929).invoke();
    return currentProject();
  }

  void Reaper::showConsoleMessage(const string& msg) {
    reaper::ShowConsoleMsg(msg.c_str());
  }

  void Reaper::clearConsole() {
    reaper::ClearConsole();
  }

  MessageBoxResult Reaper::showMessageBox(const std::string& msg, const std::string& title, MessageBoxType type) {
    return static_cast<MessageBoxResult>(reaper::ShowMessageBox(msg.c_str(), title.c_str(), static_cast<int>(type)));
  }

  Project Reaper::currentProject() const {
    return Project(reaper::EnumProjects(-1, nullptr, 0));
  }

  int Reaper::projectCount() const {
    for (int i = 0; true; i++) {
      auto reaProject = reaper::EnumProjects(i, nullptr, 0);
      if (reaProject == nullptr) {
        return i;
      }
    }
  }

  Reaper::Command::Command(int commandIndex, std::string description, std::function<void()> operation,
      std::function<bool()> isOn) : description_(description), operation_(operation), isOn_(isOn) {
    acceleratorRegister_.desc = description_.c_str();
    acceleratorRegister_.accel.cmd = (WORD) commandIndex;
  }

  Reaper::Command::Command(const Reaper::Command&& that)  : description_(std::move(that.description_)),
      operation_(std::move(that.operation_)), isOn_(std::move(that.isOn_)),
      acceleratorRegister_(std::move(that.acceleratorRegister_)) {
    // We must not let the old acceleratorRegister point to the address of the string which doesn't exist anymore
    acceleratorRegister_.desc = description_.c_str();
  }

  Guid Reaper::generateGuid() const {
    GUID guid;
    reaper::genGuid(&guid);
    return Guid(guid);
  }

  std::string Reaper::exePath() const {
    return reaper::GetExePath();
  }

  void Reaper::Command::registerIt() {
    reaper::plugin_register("gaccel", &acceleratorRegister_);
  }

  void Reaper::Command::unregister() {
    reaper::plugin_register("-gaccel", &acceleratorRegister_);
  }

  void Reaper::Command::execute() {
    operation_();
  }

  observable<Project> Reaper::projects() const {
    return observable<>::create<Project>([](subscriber<Project> s) {
      for (int i = 0; s.is_subscribed(); i++) {
        auto reaProject = reaper::EnumProjects(i, nullptr, 0);
        if (reaProject == nullptr) {
          // No more projects
          s.on_completed();
          break;
        } else {
          auto project = Project(reaProject);
          s.on_next(project);
        }
      }
    });
  }

  rxcpp::observable<MidiInputDevice> Reaper::midiInputDevices() const {
    return observable<>::create<MidiInputDevice>([this](subscriber<MidiInputDevice> s) {
      const int maxCount = reaper::GetMaxMidiInputs();
      for (int i = 0; i < maxCount && s.is_subscribed(); i++) {
        const auto dev = midiInputDeviceById(i);
        if (dev.isAvailable()) {
          s.on_next(dev);
        }
      }
      s.on_completed();
    });
  }

  MidiInputDevice Reaper::midiInputDeviceById(int id) const {
    return MidiInputDevice(id);
  }

  optional<Project> Reaper::projectByIndex(int index) const {
	auto reaProject = reaper::EnumProjects(index, nullptr, 0);
    if (reaProject == nullptr) {
      return none;
    } else {
      return Project(reaProject);
    }
  }

  AutomationMode Reaper::globalAutomationOverride() const {
    return static_cast<AutomationMode>(reaper::GetGlobalAutomationOverride());
  }

  Section Reaper::mainSection() const {
    return Section(reaper::SectionFromUniqueID(0));
  }

  Action Reaper::actionByCommandName(string commandName) const {
    return Action(commandName);
  }

  rxcpp::observable<IncomingMidiEvent> Reaper::incomingMidiEvents() const {
    return incomingMidiEventsSubject_.get_observable();
  }

  rxcpp::subscription Reaper::executeInMainThread(std::function<void(void)> command) {
    return HelperControlSurface::instance().enqueueCommand(command);
  }

  void Reaper::processAudioBuffer(bool isPost, int len, double srate, struct audio_hook_register_t* reg) {
    if (!isPost) {
      auto& reaper = Reaper::instance();
      // For each open MIDI device
      auto subject = reaper.incomingMidiEventsSubject_;
      
      for (int i = 0; subject.get_subscriber().is_subscribed() && i < reaper::GetMaxMidiInputs(); i++) {
        // Read MIDI messages
        const auto midiInput = reaper::GetMidiInput(i);
        if (midiInput != nullptr) {
          const auto midiEvents = midiInput->GetReadBuf();
          MIDI_event_t* midiEvent;
          int l = 0;
          while (subject.get_subscriber().is_subscribed() && (midiEvent = midiEvents->EnumItems(&l))) {
            // Send MIDI message
            auto& msg = midiEvent->midi_message;
            if (msg[0] != 254) {
              // No active sensing, good to go
              subject.get_subscriber().on_next(IncomingMidiEvent(MidiInputDevice(i), MidiMessage(*midiEvent)));
            }
          }
        }
      }
    }
  }
}


