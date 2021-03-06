#pragma once

#include <string>
#include <functional>
#include <memory>

#ifdef _WIN32

#include <windows.h>
#include <shlobj.h>
#include <sys/stat.h>

#else
#include <stdlib.h>
#include "swell/swell.h"
#endif

namespace reaplus {
  // DONE-rust
  std::string convertGuidToString(const GUID& guid);

  /**
   * Executes the given fillBuffer function and converts the filled buffer to a string.
   */
  // DONE-rust
  std::string toString(int maxSize, std::function<void(char*, int)> fillBuffer);

  /**
   * Executes the given fillBuffer function and converts the filled buffer to a shared string.
   */
  // DONE-rust
  std::shared_ptr<std::string> toSharedString(int maxSize, std::function<void(char*, int)> fillBuffer);
}