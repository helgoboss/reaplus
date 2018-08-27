#pragma once

namespace reaplus {
  class RegisteredAction {
  private:
    int commandIndex_;
  public:
    explicit RegisteredAction(int commandIndex);

    // TODO Use RAII
    void unregister();
  };
}

