#pragma once

namespace reaplus {
  class RegisteredAction {
  private:
    int commandIndex_;
  public:
    RegisteredAction(int commandIndex);

    void unregister();
  };
}

