#include "UndoBlock.h"
#include <reaper_plugin_functions.h>

namespace reaplus {
  bool UndoBlock::UNDO_BLOCK_ACTIVE = false;

  UndoBlock::UndoBlock(const std::string& label, ReaProject* reaProject) : label_(label), reaProject_(reaProject) {
    if (UNDO_BLOCK_ACTIVE) {
      isOuterUndoBlock_ = false;
    } else {
      isOuterUndoBlock_ = true;
      UNDO_BLOCK_ACTIVE = true;
      reaper::Undo_BeginBlock2(reaProject_);
    }
  }

  UndoBlock::~UndoBlock() {
    if (isOuterUndoBlock_) {
      reaper::Undo_EndBlock2(reaProject_, label_.c_str(), -1);
      UNDO_BLOCK_ACTIVE = false;
    }
  }
}