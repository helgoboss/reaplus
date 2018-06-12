#pragma once

#include <functional>
#include <string>
#include <rxcpp/rx.hpp>

namespace reaplus {
  class Track;
  class ReaPlusIntegrationTest {
  private:
    bool stopOnFailure_;
  public:
    ReaPlusIntegrationTest(bool stopOnFailure);
    void execute() const;
    void tests() const;
  private:
    static void assertTrue(bool expression, const std::string& errorMsg = "");
    static Track firstTrack();
    void test(const std::string& name, std::function<void(void)> code) const;
    void testWithLifetime(const std::string& name, std::function<void(rxcpp::composite_subscription)> code) const;
    void testWithUntil(const std::string& name, std::function<void(rxcpp::observable<bool>)> code) const;
    void log(const std::string& msg) const;
  };
}

