#pragma once

namespace reaplus {
  class RegisteredAction {
  private:
    int commandIndex_;
  public:
    explicit RegisteredAction(int commandIndex);

    void unregister();
  };
}

