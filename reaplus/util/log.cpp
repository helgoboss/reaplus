#include "log.h"
#include <boost/exception/diagnostic_information.hpp>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/msvc_sink.h>
#include "spdlog/details/null_mutex.h"
#include "ReaperConsoleLogSink.h"

using std::vector;
using std::shared_ptr;
using spdlog::details::null_mutex;
using spdlog::sinks::stdout_sink_st;
using spdlog::sinks::msvc_sink_st;
using spdlog::sinks::sink;
using spdlog::sinks_init_list;
using std::make_shared;

namespace {
  std::once_flag ONCE_FLAG;
}

namespace reaplus::util {
  spdlog::sinks_init_list& getDefaultLogSinks() {
    // Create sinks
    // TODO Log to file in user home instead of to console
    static auto consoleSink = make_shared<stdout_sink_st>();
    static auto reaperSink = make_shared<ReaperConsoleLogSink<null_mutex>>();
#if defined(_WIN32) && defined(HELGOBOSS_DEBUG)
    static auto debugSink = make_shared<msvc_sink_st>();
#endif
    // Create vector of all sinks
    static sinks_init_list defaultLogSinks{
        consoleSink,
#if defined(_WIN32) && defined(HELGOBOSS_DEBUG)
        debugSink,
#endif
        reaperSink
    };
    // Configure sinks (only once)
    std::call_once(ONCE_FLAG, []() {
      reaperSink->set_level(spdlog::level::err);
    });
    // Return
    return defaultLogSinks;
  }

  spdlog::logger& getMainLogger() {
    static spdlog::logger mainLogger("main", getDefaultLogSinks());
    return mainLogger;
  }

  void log(const std::string& msg) {
    // TODO Async logging: https://github.com/gabime/spdlog/wiki/6.-Asynchronous-logging
    // TODO Create per-service loggers https://github.com/gabime/spdlog/issues/630
    getMainLogger().info(msg);
  }

  void logException() {
    getMainLogger().error(boost::current_exception_diagnostic_information());
  }

  std::function<void(std::exception_ptr)>& getLoggingErrorHandler() {
    static std::function<void(std::exception_ptr)> loggingErrorHandler = [](auto ep) {
      util::logException();
    };
    return loggingErrorHandler;
  }

  std::function<void(std::exception_ptr)>& getRethrowErrorHandler() {
    static std::function<void(std::exception_ptr)> rethrowErrorHandler = [](auto ep) {
      if (ep) {
        std::rethrow_exception(ep);
      }
    };
    return rethrowErrorHandler;
  }
}