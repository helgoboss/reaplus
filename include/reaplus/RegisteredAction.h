#pragma once

namespace reaplus {
  // DONE-rust
  class RegisteredAction {
  private:
    int commandIndex_;
  public:
    explicit RegisteredAction(int commandIndex);

    // TODO Use RAII
    // DONE-rust
    void unregister();
  };
}

