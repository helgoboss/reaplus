#pragma once

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <spdlog/logger.h>
#include <spdlog/common.h>

namespace reaplus::util {
  // DONE-rust
  spdlog::sinks_init_list& getDefaultLogSinks();
  // DONE-rust
  spdlog::logger& getMainLogger();
  // DONE-rust
  void log(const std::string& msg);
  // DONE-rust
  void logException();
  // DONE-rust
  std::function<void(std::exception_ptr)>& getLoggingErrorHandler();
  // DONE-rust
  std::function<void(std::exception_ptr)>& getRethrowErrorHandler();
}