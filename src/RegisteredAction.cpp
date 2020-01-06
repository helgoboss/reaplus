#include <reaplus/RegisteredAction.h>
#include <reaplus/Reaper.h>

namespace reaplus {
  RegisteredAction::RegisteredAction(int commandIndex) : commandIndex_(commandIndex) {
  }

  void RegisteredAction::unregister() {
    auto& commandByIndex = Reaper::instance().commandByIndex_;
    if (commandByIndex.count(commandIndex_)) {
      auto& command = commandByIndex.at(commandIndex_);
      command.unregister();
      commandByIndex.erase(commandIndex_);
    }
  }
}