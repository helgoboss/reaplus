#pragma once

#include <spdlog/sinks/base_sink.h>
#include <reaper_plugin_functions.h>

namespace reaplus::util {

  template<typename Mutex>
  class ReaperConsoleLogSink : public spdlog::sinks::base_sink<Mutex> {
  protected:
    void _sink_it(const spdlog::details::log_msg& msg) override {
      const auto baseMsg = msg.formatted.str() + "\n\n";
      if (msg.level == spdlog::level::err) {
        const std::string prefix =
            "Sorry, an error occurred in a Helgoboss Projects REAPER extension. It seems that a crash has been prevented but better save your project at this point, just to be sure.\n"
            "Please report this error. Knowing about its existence and how it was caused is the single most important requirement for us to provide a fix in future versions of this extension.\n"
            "\n"
            "1. Copy'n'paste the following error information.\n"
            "2. Send the error information (and if possible also the RPP file) via email to info@helgoboss.org.\n"
            "\n"
            "Thank you for your support!\n\n";
        const auto completeMsg = prefix + baseMsg;
        reaper::ShowConsoleMsg(completeMsg.c_str());
      } else {
        reaper::ShowConsoleMsg(baseMsg.c_str());
      }
    }

    void _flush() override {
    }
  };
}