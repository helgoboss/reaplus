#pragma once

#include <string>
#include <functional>
#include <reaper_plugin.h>
#include "Parameter.h"
#include "Section.h"
#include "Reaper.h"
#include <boost/optional.hpp>

namespace reaplus {

  enum class ActionCharacter {
    Trigger,
    Toggle
  };

  class Action : public Parameter {
    friend class Section;
    friend class Reaper;
  private:
    // DONE-rust
    struct RuntimeData {
      // DONE-rust
      RuntimeData(Section section, long commandId, boost::optional<int> cachedIndex);
      Section section;
      // Sometimes shortly named cmd in REAPER API. Unique within section. Might be filled lazily.
      // For built-in actions this ID is globally stable and will always be found. For custom actions, this ID is only
      // stable at runtime and it might be that it can't be found - which means the action is not available.
      long commandId;
      boost::optional<int> cachedIndex;
    };
    // DONE-rust
    mutable boost::optional<RuntimeData> runtimeData_;
    // DONE-rust
    // Used to represent custom actions that are not available (they don't have a commandId) or for which is not yet
    // known if they are available. Globally unique, not within one section.
    // TODO But currently only mainSection supported. How support other sections?
    boost::optional<std::string> commandName_;
  public:
    std::unique_ptr<Parameter> clone() const override;

    // DONE-rust
    ParameterType parameterType() const override;

    // DONE-rust
    long commandId() const;

    // TODO Return optional, don't automatically convert commandId() into string!!!
    // DONE-rust
    std::string commandName() const;

    // Returns empty string if action disappeared
    // DONE-rust
    std::string name() const;

    // DONE-rust
    int index() const;

    // DONE-rust
    void invoke(boost::optional<Project> project = boost::none);

    // DONE-rust
    void invoke(double normalizedValue, bool isStepCount, boost::optional<Project> project = boost::none);

    // DONE-rust
    bool isOn() const;

    // DONE-rust
    ActionCharacter character() const;

    // DONE-rust
    Section section() const;

    // DONE-rust
    bool isAvailable() const;

    // DONE-rust
    friend bool operator==(const Action& lhs, const Action& rhs);

  protected:
    // DONE-rust
    Action(Section section, long commandId, boost::optional<int> index);

    // DONE-rust
    explicit Action(std::string commandName);

  private:
    static bool containsDigitsOnly(const std::string& text);
    // DONE-rust
    static std::string fixCommandName(const std::string& commandName);

    // DONE-rust
    bool equals(const Parameter& other) const override;

    // DONE-rust
    boost::optional<int> findIndex() const;

    // DONE-rust
    bool loadByCommandName() const;

    // DONE-rust
    void loadIfNecessaryOrComplain() const;
  };
}
