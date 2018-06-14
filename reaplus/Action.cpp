#include <reaper_plugin.h>
#include <stdexcept>
#include <regex>
#include <cstring>
#include "Action.h"
#include "ModelUtil.h"
#include "HelperControlSurface.h"
#include "reaper_plugin_functions.h"
#undef min
#undef max

using rxcpp::subscriber;
using boost::none;
using std::string;
using std::unique_ptr;

namespace reaplus {
  static std::regex NON_DIGIT_REGEX("[^0-9]");

  Action::Action(Section section, long commandId, boost::optional<int> index) : commandName_(none),
      runtimeData_(RuntimeData(section, commandId, index)) {
  }

  long Action::commandId() const {
    loadIfNecessaryOrComplain();
    return runtimeData_->commandId;
  }

  std::string Action::name() const {
    loadIfNecessaryOrComplain();
    return std::string(
        reaper::kbd_getTextFromCmd((DWORD) runtimeData_->commandId, runtimeData_->section.sectionInfo()));
  }

  std::string Action::commandName() const {
    if (commandName_) {
      return *commandName_;
    } else {
      const auto nameCString = reaper::ReverseNamedCommandLookup(runtimeData_->commandId);
      if (nameCString == nullptr) {
        return std::to_string(runtimeData_->commandId);
      } else {
        return std::string(nameCString);
      }
    }
  }

  int Action::index() const {
    loadIfNecessaryOrComplain();
    if (!runtimeData_->cachedIndex.is_initialized()) {
      runtimeData_->cachedIndex = findIndex();
    }
    return *runtimeData_->cachedIndex;
  }

  boost::optional<int> Action::findIndex() const {
    // TODO Use kbd_enumerateActions
    for (int i = 0; i < runtimeData_->section.sectionInfo()->action_list_cnt; i++) {
      const KbdCmd kbdCmd = runtimeData_->section.sectionInfo()->action_list[i];
      if (kbdCmd.cmd == runtimeData_->commandId) {
        return i;
      }
    }
    return none;
  }

  void Action::invoke(double normalizedValue, bool isStepCount, boost::optional<Project> project) {
    // TODO I have no idea how to launch an action in a specific section. The first function doesn't seem to launch the action :(
    // bool (*kbd_RunCommandThroughHooks)(KbdSectionInfo* section, int* actionCommandID, int* val, int* valhw, int* relmode, HWND hwnd);
    // int (*KBD_OnMainActionEx)(int cmd, int val, int valhw, int relmode, HWND hwnd, ReaProject* proj);
    loadIfNecessaryOrComplain();
    int actionCommandId = runtimeData_->commandId;
    if (isStepCount) {
      const int relativeValue = 64 + (int) normalizedValue;
      const int croppedRelativeValue = std::max(std::min(relativeValue, 127), 0);
      int val = croppedRelativeValue;
      int valhw = 0;
      int relmode = 2;
      // reaper::kbd_RunCommandThroughHooks(section_.sectionInfo(), &actionCommandId, &val, &valhw, &relmode, reaper::GetMainHwnd());
      reaper::KBD_OnMainActionEx(
          actionCommandId,
          val,
          valhw,
          relmode,
          reaper::GetMainHwnd(),
          project ? project->reaProject() : nullptr
      );
    } else {
      int val = static_cast<int>(std::round(normalizedValue * 127));
      int valhw = -1;
      int relmode = 0;
      // reaper::kbd_RunCommandThroughHooks(section_.sectionInfo(), &actionCommandId, &val, &valhw, &relmode, reaper::GetMainHwnd());
      reaper::KBD_OnMainActionEx(
          actionCommandId,
          val,
          valhw,
          relmode,
          reaper::GetMainHwnd(),
          project ? project->reaProject() : nullptr
      );
      // Main_OnCommandEx would trigger the actionInvoked event but it has not enough parameters for passing values etc.
//      reaper::Main_OnCommandEx(actionCommandId, 0, project ? project->reaProject() : nullptr);
    }
  }

  void Action::invoke(boost::optional<Project> project) {
    invoke(1, false, project);
  }

  bool Action::isOn() const {
    loadIfNecessaryOrComplain();
    return reaper::GetToggleCommandState2(runtimeData_->section.sectionInfo(), runtimeData_->commandId) == 1;
  }

  ActionCharacter Action::character() const {
    loadIfNecessaryOrComplain();
    if (reaper::GetToggleCommandState2(runtimeData_->section.sectionInfo(), runtimeData_->commandId) == -1) {
      return ActionCharacter::Trigger;
    } else {
      return ActionCharacter::Toggle;
    }
  }

  bool reaplus::operator==(const Action& lhs, const Action& rhs) {
    if (lhs.runtimeData_ && rhs.runtimeData_) {
      return lhs.runtimeData_->section == rhs.runtimeData_->section
          && lhs.runtimeData_->commandId == rhs.runtimeData_->commandId;
    } else {
      return lhs.commandName_ && lhs.commandName_ == rhs.commandName_;
    }
  }

  ParameterType Action::parameterType() const {
    return ParameterType::Action;
  }

  bool Action::equals(const Parameter& other) const {
    auto& o = static_cast<const Action&>(other);
    // TODO Do we still need the specialized == operator implementation if we already have the polymorphic one?
    return *this == o;
  }

  unique_ptr<Parameter> Action::clone() const {
    return unique_ptr<Action>(new Action(*this));
  }

  Section Action::section() const {
    loadIfNecessaryOrComplain();
    return runtimeData_->section;
  }

  Action::Action(string commandName) : commandName_(commandName), runtimeData_(none) {

  }

  bool Action::isAvailable() const {
    if (runtimeData_) {
      // See if we can get a description. If yes, the action actually exists. If not, then not.
      const auto text =
          reaper::kbd_getTextFromCmd((DWORD) runtimeData_->commandId, runtimeData_->section.sectionInfo());
      return text != nullptr && std::strlen(text) > 0;
    } else {
      return loadByCommandName();
    }
  }

  bool Action::loadByCommandName() const {
    const string fixedCommandName = Action::fixCommandName(*commandName_);
    const int id = reaper::NamedCommandLookup(fixedCommandName.c_str());
    if (id == 0) {
      return false;
    } else {
      runtimeData_ = RuntimeData(Reaper::instance().mainSection(), id, none);
      return true;
    }
  }

  Action::RuntimeData::RuntimeData(Section section, long commandId, boost::optional<int> cachedIndex) :
      section(section), commandId(commandId), cachedIndex(cachedIndex) {

  }

  string Action::fixCommandName(const string& commandName) {
    if (!commandName.empty() && commandName[0] == '_') {
      // Command already contains underscore. Great.
      return commandName;
    } else if (commandName.size() == 32 || !containsDigitsOnly(commandName)) {
      // Doesn't contain underscore but should contain one because it's a custom action or an explicitly named command.
      return std::string("_") + commandName;
    } else {
      // Is empty or contains digits. Return as is.
      return commandName;
    }
  }
  bool Action::containsDigitsOnly(const string& text) {
    return !std::regex_search(text, NON_DIGIT_REGEX);
  }

  void Action::loadIfNecessaryOrComplain() const {
    if (!runtimeData_ && !loadByCommandName()) {
      throw std::logic_error("Action not loadable");
    }
  }

}
