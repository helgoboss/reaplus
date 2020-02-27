#pragma once

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>

namespace reaplus::util {

  // DONE-rust
  class ReaperConsoleLogSink : public spdlog::sinks::base_sink<spdlog::details::null_mutex> {
  protected:
    void sink_it_(const spdlog::details::log_msg& msg) override;

    void flush_() override;
  };
}