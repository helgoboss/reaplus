#include "Section.h"
#include "Action.h"
#include "reaper_plugin_functions.h"

using rxcpp::observable;
using rxcpp::subscriber;
using boost::optional;
using boost::none;

namespace reaplus {
  rxcpp::observable<Action> Section::actions() const {
    return rxcpp::observable<>::create<Action>([this](subscriber<Action> s) {
      for (int i = 0; i < actionCount() && s.is_subscribed(); i++) {
        const auto kbdCmd = sectionInfo_->action_list[i];
        s.on_next(Action(*this, kbdCmd.cmd, i));
      }
      s.on_completed();
    });
  }

  int Section::actionCount() const {
    return sectionInfo_->action_list_cnt;
  }

  Section::Section(KbdSectionInfo* sectionInfo) : sectionInfo_(sectionInfo) {

  }

  KbdSectionInfo* Section::sectionInfo() const {
    return sectionInfo_;
  }

  Action Section::actionByCommandId(int commandId) const {
    return Action(*this, commandId, none);
  }

  bool reaplus::operator==(const Section& lhs, const Section& rhs) {
    return lhs.sectionInfo_ == rhs.sectionInfo_;
  }
}