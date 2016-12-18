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

  class Action: public Parameter {
    friend class Section;
    friend class Reaper;
  private:
    struct RuntimeData {
      RuntimeData(Section section, long commandId, boost::optional<int> cachedIndex);
      Section section;
      // Sometimes shortly named cmd in REAPER API. Unique within section. Might be filled lazily.
      // For built-in actions this ID is globally stable and will always be found. For custom actions, this ID is only
      // stable at runtime and it might be that it can't be found - which means the action is not available.
      long commandId;
      boost::optional<int> cachedIndex;
    };
    mutable boost::optional<RuntimeData> runtimeData_;
    // Used to represent custom actions that are not available (they don't have a commandId) or for which is not yet
    // known if they are available. Globally unique, not within one section.
    // TODO But currently only mainSection supported. How support other sections?
    boost::optional<std::string> commandName_;
  public:
    virtual std::unique_ptr<Parameter> clone() const override;

    virtual ParameterType parameterType() const override;

    long commandId() const;

    // TODO Return optional, don't automatically convert commandId() into string!!!
    std::string commandName() const;

    // Returns empty string if action disappeared
    std::string name() const;

    int index() const;

    void invoke(boost::optional<Project> project = boost::none);

    void invoke(double normalizedValue, bool isStepCount, boost::optional<Project> project = boost::none);

    bool isOn() const;

    ActionCharacter character() const;

    Section section() const;

    bool isAvailable() const;

    friend bool operator==(const Action& lhs, const Action& rhs);

  protected:
    Action(Section section, long commandId, boost::optional<int> index);

    Action(std::string commandName);

  private:
    static bool containsDigitsOnly(const std::string& text);
    static std::string fixCommandName(const std::string& commandName);

    virtual bool equals(const Parameter& other) const override;

    boost::optional<int> findIndex() const;

    bool loadByCommandName() const;

    void loadIfNecessaryOrComplain() const;
  };
}
