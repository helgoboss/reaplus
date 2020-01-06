#pragma once

#include <string>
#include "reaper_plugin.h"

namespace reaplus {
  class Project;
  // Constructor takes care of starting the undo block. Destructor takes care of ending the undo block (RAII).
  // Doesn't start a new block if we already are in an undo block.
  class UndoBlock {
    friend class Project;
  protected:
    UndoBlock(const std::string& label, ReaProject* reaProject);
    ~UndoBlock();

  private:
    static bool UNDO_BLOCK_ACTIVE;
    bool isOuterUndoBlock_;
    std::string label_;
    ReaProject* reaProject_;
  };
}

