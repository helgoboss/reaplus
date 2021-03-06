#pragma once

#include <functional>
#include <string>
#include <rxcpp/rx.hpp>
#include <queue>

namespace reaplus {
  class Track;
  // DONE-rust
  class TestStep {
  private:
    using Operation = std::function<rxcpp::observable<bool>(rxcpp::observable<bool>)>;
    const std::string name_;
    const Operation operation_;
  public:
    TestStep(std::string name, Operation operation);
    std::string getName() const;
    Operation getOperation() const;
  };

  class ReaPlusIntegrationTest {
  private:
    std::queue<TestStep> stepQueue_;
  public:
    // DONE-rust
    void execute();
  private:
    static void assertTrue(bool expression, const std::string& errorMsg = "", const std::string& minReaperVersion = "");
    static Track firstTrack();
    static Track secondTrack();
    // DONE-rust
    void tests();
    void executeNextStep();
    void test(const std::string& name, std::function<void(void)> code);
    void testWithUntil(const std::string& name, std::function<void(rxcpp::observable<bool>)> code);
    void testAndWait(const std::string& name, std::function<rxcpp::observable<bool>(void)> code);
    void testInternal(const std::string& name,
        std::function<rxcpp::observable<bool>(rxcpp::observable<bool>)> code);
    // DONE-rust
    void log(const std::string& msg);
    // DONE-rust
    void logHeading(const std::string& name);
    // DONE-rust
    void processSuccess();
    // DONE-rust
    void processFailure(const std::exception& e);
  };
}

