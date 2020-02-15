#pragma once

#include <rxcpp/rx.hpp>
#include <reaper_plugin.h>
#include <boost/optional.hpp>
#include "Reaper.h"

namespace reaplus {
  class Action;
  class Section {
    friend class Reaper;
  private:
    KbdSectionInfo* sectionInfo_;
  public:
    // DONE-rust
    int actionCount() const;
    // DONE-rust
    rxcpp::observable<Action> actions() const;
    // DONE-rust
    KbdSectionInfo* sectionInfo() const;
    // DONE-rust
    Action actionByCommandId(int commandId) const;
    // DONE-rust
    Action actionByIndex(int index) const;
    // DONE-rust
    friend bool operator==(const Section& lhs, const Section& rhs);
  protected:
    // DONE-rust
    explicit Section(KbdSectionInfo* sectionInfo);
  private:
    // DONE-rust
    Action actionByIndexUnchecked(int index) const;
  };
}

