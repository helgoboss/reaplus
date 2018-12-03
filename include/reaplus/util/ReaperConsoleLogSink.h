#pragma once

#include <spdlog/sinks/base_sink.h>
#include <spdlog/details/null_mutex.h>

namespace reaplus::util {

  class ReaperConsoleLogSink : public spdlog::sinks::base_sink<spdlog::details::null_mutex> {
  protected:
    void _sink_it(const spdlog::details::log_msg& msg) override;

    void _flush() override;
  };
}