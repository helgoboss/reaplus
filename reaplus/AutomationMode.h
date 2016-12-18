#pragma once

namespace reaplus {
  enum class AutomationMode: int {
    NoOverride = -1,
    TrimRead = 0,
    Read = 1,
    Touch = 2,
    Write = 3,
    Latch = 4,
    Bypass = 5
  };
}

