#pragma once

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include <spdlog/logger.h>
#include <spdlog/common.h>

namespace reaplus::util {
  spdlog::sinks_init_list& getDefaultLogSinks();
  spdlog::logger& getMainLogger();
  void log(const std::string& msg);
  void logException();
  std::function<void(std::exception_ptr)>& getLoggingErrorHandler();
  std::function<void(std::exception_ptr)>& getRethrowErrorHandler();
}