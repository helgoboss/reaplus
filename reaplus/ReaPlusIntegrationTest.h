#pragma once

#include <functional>
#include <string>

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
    void log(const std::string& msg) const;
  };
}

