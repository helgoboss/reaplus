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
    int actionCount() const;
    rxcpp::observable<Action> actions() const;
    KbdSectionInfo* sectionInfo() const;
    Action actionByCommandId(int commandId) const;
    friend bool operator==(const Section& lhs, const Section& rhs);
  protected:
    Section(KbdSectionInfo* sectionInfo);
  };
}

