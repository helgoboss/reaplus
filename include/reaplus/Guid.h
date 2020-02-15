#pragma once

#include <reaper_plugin.h>
#include <string>

namespace reaplus {
  // DONE-rust
  class Guid {
  private:
    GUID data_;
  public:
    explicit Guid(GUID data);
    GUID data() const;
    std::string toString() const;
  };
}

