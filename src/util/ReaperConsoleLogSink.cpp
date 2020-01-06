#include <reaplus/util/ReaperConsoleLogSink.h>
#include <reaplus/Reaper.h>

namespace reaplus::util {
  void ReaperConsoleLogSink::sink_it_(const spdlog::details::log_msg& msg) {
    spdlog::memory_buf_t formatted;
    base_sink<spdlog::details::null_mutex>::formatter_->format(msg, formatted);
    const auto baseMsg = fmt::to_string(formatted) + "\n\n";
    const auto level = msg.level;
    Reaper::instance().executeWhenInMainThread([baseMsg, level] {
      if (level == spdlog::level::err) {
        const std::string prefix =
            "Sorry, an error occurred in a Helgoboss Projects REAPER extension. It seems that a crash has been prevented, but better save your project at this point, just to be sure.\n"
            "Please report this error. Knowing about its existence and how it was caused is the single most important requirement for us to provide a fix in future versions of this extension.\n"
            "\n"
            "1. Copy the following error information.\n"
            "2. Paste the error information into an email and send it via email to info@helgoboss.org, along with the RPP file, your REAPER.ini file and some instructions how to reproduce the issue.\n"
            "\n"
            "Thank you for your support!\n\n";
        const auto completeMsg = prefix + "--- cut ---\n" + baseMsg + "\n--- cut ---\n";
        Reaper::instance().showConsoleMessage(completeMsg);
      } else {
        Reaper::instance().showConsoleMessage(baseMsg);
      }
    });
  }

  void ReaperConsoleLogSink::flush_() {
  }
}