#include <reaplus/HelperControlSurface.h>
#include <reaper_plugin.h>
#include <reaplus/Reaper.h>
#include <reaplus/TrackSend.h>
#include <reaplus/FxParameter.h>
#include <reaplus/Project.h>
#include <reaplus/Section.h>
#include <reaplus/Action.h>
#include <reaplus/MidiInputDevice.h>
#include <reaplus/MidiOutputDevice.h>
#include <reaplus/IncomingMidiEvent.h>
#include <reaplus/HelperControlSurface.h>
#include <reaplus/util/log.h>
#include <reaper_plugin_functions.h>
#include <utility>

using rxcpp::subscriber;
using boost::none;
using std::string;
using std::string;
using std::function;
using std::unordered_map;
using std::unique_ptr;
using rxcpp::observable;
using boost::optional;
using helgoboss::MidiMessage;

namespace reaplus {
  std::unique_ptr<Reaper> Reaper::INSTANCE = nullptr;

  Reaper& reaplus::Reaper::instance() {
    static Guard guard;
    if (INSTANCE == nullptr) {
      INSTANCE = unique_ptr<Reaper>(new Reaper());
    }
    return *INSTANCE;
  }

  void Reaper::destroyInstance() {
    if (INSTANCE != nullptr) {
      INSTANCE = nullptr;
    }
  }

  Reaper::Guard::~Guard() {
    Reaper::destroyInstance();
  }

  RegisteredAction Reaper::registerAction(const string& commandId, const string& description,
      function<void()> operation, function<bool()> isOn) {
    const int commandIndex = reaper::plugin_register("command_id", (void*) commandId.c_str());
    auto pair = commandByIndex_.emplace(
        commandIndex,
        Command(commandIndex, description, std::move(operation), std::move(isOn))
    );
    Command& command = pair.first->second;
    command.registerIt();
    return RegisteredAction(commandIndex);
  }

  bool Reaper::staticHookCommand(int commandIndex, int) {
    auto& commandByIndex = Reaper::instance().commandByIndex_;
    if (commandByIndex.count(commandIndex)) {
      auto& command = commandByIndex.at(commandIndex);
      command.execute();
      return true;
    } else {
      return false;
    }
  }

  void Reaper::staticHookPostCommand(int commandId, int) {
    auto action = Reaper::instance().mainSection().actionByCommandId(commandId);
    instance().actionInvokedSubject_.get_subscriber().on_next(action);
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

  rxcpp::observable<Action> Reaper::actionInvoked() const {
    return actionInvokedSubject_.get_observable();
  }

  Reaper::Reaper() {
    // DONE-rust
    idOfMainThread_ = std::this_thread::get_id();
    // TODO-rust
    projectConfigExtension_.ProcessExtensionLine = &processExtensionLine;
    // TODO-rust
    projectConfigExtension_.BeginLoadProjectState = &beginLoadProjectState;
    // TODO-rust
    projectConfigExtension_.SaveExtensionConfig = &saveExtensionConfig;
    // TODO-rust
    reaper::plugin_register("projectconfig", (void*) &projectConfigExtension_);
    // DONE-rust
    reaper::plugin_register("hookcommand", (void*) &staticHookCommand);
    // DONE-rust
    reaper::plugin_register("toggleaction", (void*) &staticToggleAction);
    // DONE-rust
    reaper::plugin_register("hookpostcommand", (void*) &staticHookPostCommand);
    // DONE-rust
    audioHook_.OnAudioBuffer = &processAudioBuffer;
    // DONE-rust
    reaper::Audio_RegHardwareHook(true, &audioHook_);
  }

  Reaper::~Reaper() {
    // TODO-rust
    HelperControlSurface::destroyInstance();
    // DONE-rust
    reaper::Audio_RegHardwareHook(false, &audioHook_);
    // DONE-rust
    reaper::plugin_register("-hookpostcommand", (void*) &staticHookPostCommand);
    // DONE-rust
    reaper::plugin_register("-toggleaction", (void*) &staticToggleAction);
    // DONE-rust
    reaper::plugin_register("-hookcommand", (void*) &staticHookCommand);
    // TODO-rust
    reaper::plugin_register("-projectconfig", (void*) &projectConfigExtension_);
  }

  optional<FxParameter> Reaper::lastTouchedFxParameter() const {
    // TODO Sucks: We have to assume it was a parameter in the current project
    // Maybe we should rather rely on our own technique in ControlSurface here!
    int trackIndex;
    int fxQueryIndex;
    int paramIndex;
    // fxQueryIndex is only a real query index since REAPER 5.95, before it didn't say if it's input FX or normal one!
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
      std::function<bool()> isOn) : description_(std::move(description)), operation_(std::move(operation)),
      isOn_(std::move(isOn)) {
    acceleratorRegister_.desc = description_.c_str();
    acceleratorRegister_.accel.cmd = (WORD) commandIndex;
  }

  Reaper::Command::Command(Reaper::Command&& that) noexcept :
      description_(std::move(that.description_)),
      operation_(std::move(that.operation_)),
      isOn_(std::move(that.isOn_)),
      acceleratorRegister_(that.acceleratorRegister_) {
    // We must not let the old acceleratorRegister point to the address of the string which doesn't exist anymore
    acceleratorRegister_.desc = description_.c_str();
  }

  boost::optional<Fx> Reaper::focusedFx() const {
    int trackNumber;
    int itemNumber;
    int fxNumber;
    const int result = reaper::GetFocusedFX(&trackNumber, &itemNumber, &fxNumber);
    switch (result) {
      case 1: {
        // We don't know the project so we must check each project
        return projectsWithCurrentOneFirst()
            .map([trackNumber, fxNumber](Project p) -> boost::optional<Fx> {
              if (const auto track = p.trackByIndex(trackNumber)) {
                if (const auto fx = track->normalFxChain().fxByIndex(fxNumber)) {
                  if (fx->windowHasFocus()) {
                    return fx;
                  } else {
                    return none;
                  }
                } else {
                  return none;
                }
              } else {
                return none;
              }
            })
            .default_if_empty(boost::optional<Fx>())
            .as_blocking()
            .first();
      }
      case 0:
      case 2:return none;
      default:return none;
    }
  }

  Guid Reaper::generateGuid() const {
    GUID guid;
    reaper::genGuid(&guid);
    return Guid(guid);
  }

  std::string Reaper::exePath() const {
    return reaper::GetExePath();
  }

  boost::filesystem::path Reaper::getResourceDir() const {
    return reaper::GetResourcePath();
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

  rxcpp::observable<Project> Reaper::projectsWithCurrentOneFirst() const {
    const auto currentProject = this->currentProject();
    return observable<>::just(currentProject)
        .concat(
            projects().filter([currentProject](Project p) {
              return p != currentProject;
            })
        );
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

  rxcpp::observable<MidiOutputDevice> Reaper::midiOutputDevices() const {
    return observable<>::create<MidiOutputDevice>([this](subscriber<MidiOutputDevice> s) {
      const int maxCount = reaper::GetMaxMidiOutputs();
      for (int i = 0; i < maxCount && s.is_subscribed(); i++) {
        const auto dev = midiOutputDeviceById(i);
        if (dev.isAvailable()) {
          s.on_next(dev);
        }
      }
      s.on_completed();
    });
  }

  MidiOutputDevice Reaper::midiOutputDeviceById(int id) const {
    return MidiOutputDevice(id);
  }

  optional<Project> Reaper::projectByIndex(int index) const {
    auto reaProject = reaper::EnumProjects(index, nullptr, 0);
    if (reaProject == nullptr) {
      return none;
    } else {
      return Project(reaProject);
    }
  }

  boost::optional<Project> Reaper::getCurrentlyRenderingProject() const {
    return projectByIndex(0x40000000);
  }

  AutomationMode Reaper::globalAutomationOverride() const {
    return static_cast<AutomationMode>(reaper::GetGlobalAutomationOverride());
  }

  Section Reaper::mainSection() const {
    return Section(reaper::SectionFromUniqueID(0));
  }

  Action Reaper::actionByCommandName(string commandName) const {
    return Action(std::move(commandName));
  }

  rxcpp::observable<IncomingMidiEvent> Reaper::incomingMidiEvents() const {
    return incomingMidiEventsSubject_.get_observable();
  }

  void Reaper::init() {
    HelperControlSurface::init();
  }

  uint64_t Reaper::sampleCounter() const {
    return sampleCounter_;
  }

  void Reaper::processAudioBuffer(bool isPost, int len, double, struct audio_hook_register_t*) {
    try {
      if (!isPost) {
        auto& reaper = Reaper::instance();
        // Make use of audioThreadCoordination for rxcpp possible
        // TODO-rust
        reaper.audioThreadRunLoop_.dispatch();
        // For each open MIDI device
        auto& subject = reaper.incomingMidiEventsSubject_;
        for (int i = 0; subject.has_observers() && i < reaper::GetMaxMidiInputs(); i++) {
          // Read MIDI messages
          const auto midiInput = reaper::GetMidiInput(i);
          if (midiInput != nullptr) {
            const auto midiEvents = midiInput->GetReadBuf();
            MIDI_event_t* midiEvent;
            int l = 0;
            while (subject.has_observers() && (midiEvent = midiEvents->EnumItems(&l))) {
              // Send MIDI message
              auto& msg = midiEvent->midi_message;
              if (msg[0] != 254) {
                // No active sensing, good to go
                subject.get_subscriber().on_next(IncomingMidiEvent(
                    MidiInputDevice(i),
                    createMidiMessageFromEvent(*midiEvent),
                    midiEvent->frame_offset
                ));
              }
            }
          }
        }
        reaper.sampleCounter_ += len;
      }
    } catch (...) {
      util::logException();
    }
  }

  MidiMessage Reaper::createMidiMessageFromEvent(const MIDI_event_t& event) {
    if (event.size == 0) {
      return MidiMessage::empty();
    }
    return MidiMessage(
        event.midi_message[0],
        event.size >= 2 ? event.midi_message[1] : static_cast<unsigned char>(0),
        event.size >= 3 ? event.midi_message[2] : static_cast<unsigned char>(0)
    );
  }

  void Reaper::registerProjectConfigExtension(ProjectConfigExtension extension) {
    projectConfigExtensions_.emplace_back(std::move(extension));
  }

  bool Reaper::processExtensionLine(const char* line,
      ProjectStateContext* ctx,
      bool isUndo,
      struct project_config_extension_t* reg) {
    auto& reaper = Reaper::instance();
    for (ProjectConfigExtension& extension : reaper.projectConfigExtensions_) {
      if (extension.processExtensionLine(line, ctx, isUndo, reg)) {
        return true;
      }
    }
    return false;
  }

  void Reaper::beginLoadProjectState(bool isUndo, struct project_config_extension_t* reg) {
    auto& reaper = Reaper::instance();
    for (ProjectConfigExtension& extension : reaper.projectConfigExtensions_) {
      extension.beginLoadProjectState(isUndo, reg);
    }
  }

  void Reaper::saveExtensionConfig(ProjectStateContext* ctx, bool isUndo, struct project_config_extension_t* reg) {
    auto& reaper = Reaper::instance();
    for (ProjectConfigExtension& extension : reaper.projectConfigExtensions_) {
      extension.saveExtensionConfig(ctx, isUndo, reg);
    }
  }

  rxcpp::observable<Parameter*> Reaper::parameterValueChangedUnsafe() const {
    return HelperControlSurface::instance().parameterValueChangedUnsafe();
  }

  rxcpp::observable<Parameter*> Reaper::parameterTouchedUnsafe() const {
    return HelperControlSurface::instance().parameterTouchedUnsafe();
  }

  rxcpp::observable<FxParameter> Reaper::fxParameterValueChanged() const {
    return HelperControlSurface::instance().fxParameterValueChanged();
  }

  rxcpp::observable<FxParameter> Reaper::fxParameterTouched() const {
    return HelperControlSurface::instance().fxParameterTouched();
  }

  rxcpp::observable<Fx> Reaper::fxOpened() const {
    return HelperControlSurface::instance().fxOpened();
  }

  rxcpp::observable<Fx> Reaper::fxClosed() const {
    return HelperControlSurface::instance().fxClosed();
  }

  rxcpp::observable<boost::optional<Fx>> Reaper::fxFocused() const {
    return HelperControlSurface::instance().fxFocused();
  }

  rxcpp::observable<Track> Reaper::trackVolumeChanged() const {
    return HelperControlSurface::instance().trackVolumeChanged();
  }

  rxcpp::observable<Track> Reaper::trackVolumeTouched() const {
    return HelperControlSurface::instance().trackVolumeTouched();
  }

  rxcpp::observable<Track> Reaper::trackPanChanged() const {
    return HelperControlSurface::instance().trackPanChanged();
  }

  rxcpp::observable<Track> Reaper::trackNameChanged() const {
    return HelperControlSurface::instance().trackNameChanged();
  }

  rxcpp::observable<Track> Reaper::trackInputChanged() const {
    return HelperControlSurface::instance().trackInputChanged();
  }

  rxcpp::observable<Track> Reaper::trackPanTouched() const {
    return HelperControlSurface::instance().trackPanTouched();
  }

  rxcpp::observable<Track> Reaper::trackArmTouched() const {
    return HelperControlSurface::instance().trackArmTouched();
  }

  rxcpp::observable<TrackSend> Reaper::trackSendVolumeChanged() const {
    return HelperControlSurface::instance().trackSendVolumeChanged();
  }

  rxcpp::observable<TrackSend> Reaper::trackSendVolumeTouched() const {
    return HelperControlSurface::instance().trackSendVolumeTouched();
  }

  rxcpp::observable<TrackSend> Reaper::trackSendPanChanged() const {
    return HelperControlSurface::instance().trackSendPanChanged();
  }

  rxcpp::observable<TrackSend> Reaper::trackSendPanTouched() const {
    return HelperControlSurface::instance().trackSendPanTouched();
  }

  rxcpp::observable<Track> Reaper::trackAdded() const {
    return HelperControlSurface::instance().trackAdded();
  }

  rxcpp::observable<Track> Reaper::trackRemoved() const {
    return HelperControlSurface::instance().trackRemoved();
  }

  rxcpp::observable<Project> Reaper::tracksReordered() const {
    return HelperControlSurface::instance().tracksReordered();
  }

  rxcpp::observable<Fx> Reaper::fxAdded() const {
    return HelperControlSurface::instance().fxAdded();
  }

  rxcpp::observable<Fx> Reaper::fxEnabledChanged() const {
    return HelperControlSurface::instance().fxEnabledChanged();
  }

  rxcpp::observable<Fx> Reaper::fxEnabledTouched() const {
    return HelperControlSurface::instance().fxEnabledTouched();
  }

  rxcpp::observable<Fx> Reaper::fxRemoved() const {
    return HelperControlSurface::instance().fxRemoved();
  }

  rxcpp::observable<Track> Reaper::fxReordered() const {
    return HelperControlSurface::instance().fxReordered();
  }

  rxcpp::observable<Track> Reaper::trackInputMonitoringChanged() const {
    return HelperControlSurface::instance().trackInputMonitoringChanged();
  }

  rxcpp::observable<Track> Reaper::trackArmChanged() const {
    return HelperControlSurface::instance().trackArmChanged();
  }

  rxcpp::observable<Track> Reaper::trackMuteChanged() const {
    return HelperControlSurface::instance().trackMuteChanged();
  }

  rxcpp::observable<Track> Reaper::trackMuteTouched() const {
    return HelperControlSurface::instance().trackMuteTouched();
  }

  rxcpp::observable<Track> Reaper::trackSoloChanged() const {
    return HelperControlSurface::instance().trackSoloChanged();
  }

  rxcpp::observable<Track> Reaper::trackSoloTouched() const {
    return HelperControlSurface::instance().trackSoloTouched();
  }

  rxcpp::observable<Track> Reaper::trackSelectedChanged() const {
    return HelperControlSurface::instance().trackSelectedChanged();
  }

  rxcpp::observable<Track> Reaper::trackSelectedTouched() const {
    return HelperControlSurface::instance().trackSelectedTouched();
  }

  rxcpp::observable<Project> Reaper::projectSwitched() const {
    return HelperControlSurface::instance().projectSwitched();
  }

  rxcpp::observable<Project> Reaper::projectClosed() const {
    return HelperControlSurface::instance().projectClosed();
  }

  rxcpp::observable<bool> Reaper::masterTempoChanged() const {
    return HelperControlSurface::instance().masterTempoChanged();
  }

  rxcpp::observable<bool> Reaper::masterTempoTouched() const {
    return HelperControlSurface::instance().masterTempoTouched();
  }

  rxcpp::observable<bool> Reaper::masterPlayrateChanged() const {
    return HelperControlSurface::instance().masterPlayrateChanged();
  }

  rxcpp::observable<bool> Reaper::masterPlayrateTouched() const {
    return HelperControlSurface::instance().masterPlayrateTouched();
  }

  rxcpp::observable<bool> Reaper::mainThreadIdle() const {
    return HelperControlSurface::instance().mainThreadIdle();
  }

  rxcpp::composite_subscription Reaper::executeLaterInMainThread(std::function<void(void)> command) {
    return HelperControlSurface::instance().enqueueCommand(std::move(command));
  }
  void Reaper::executeLaterInMainThreadFast(std::function<void(void)> command) {
    HelperControlSurface::instance().enqueueCommandFast(std::move(command));
  }

  rxcpp::composite_subscription Reaper::executeWhenInMainThread(std::function<void(void)> command) {
    if (currentThreadIsMainThread()) {
      command();
      return rxcpp::composite_subscription();
    } else {
      return executeLaterInMainThread(command);
    }
  }

  const rxcpp::observe_on_one_worker& Reaper::mainThreadCoordination() const {
    return HelperControlSurface::instance().mainThreadCoordination();
  }

  const rxcpp::observe_on_one_worker& Reaper::audioThreadCoordination() const {
    return audioThreadCoordination_;
  }

  std::thread::id Reaper::idOfMainThread() const {
    return idOfMainThread_;
  }

  bool Reaper::currentThreadIsMainThread() const {
    return std::this_thread::get_id() == idOfMainThread_;
  }

  HWND Reaper::mainWindow() const {
    return reaper::GetMainHwnd();
  }

  void Reaper::stuffMidiMessage(StuffMidiMessageTarget target, const MidiMessage& message) {
    const auto mode = static_cast<int>(target);
    reaper::StuffMIDIMessage(mode, message.getStatusByte(), message.getDataByte1(), message.getDataByte2());
  }

  std::string Reaper::getVersion() const {
    return reaper::GetAppVersion();
  }
}


