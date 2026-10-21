// Purpose: Implements the agent's action model: restored
// actions use the standard rule, all other actions use the
// base model (plan 3.5).
// Introduced in: Stage 3c (rebuild)
// Architecture: current() only assembles an ActionModel. It
// makes no prediction itself, so it spends no budget; the
// StepPredictor charges one evaluation each time it calls the
// assembled model. setBase is the IE-1 H2 hook.

#include "agent/agent_model.h"

#include <array>
#include <cstddef>

#include "agent/directions.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

// Turns an action into an index 0..3 following kActionOrder
// (North, East, South, West), so the restored marks can live
// in a small fixed-size array with one mark per action.
int actionIndex(Action action) {
    for (int index = 0; index < 4; index = index + 1) {
        const std::size_t at = static_cast<std::size_t>(index);
        if (kActionOrder[at] == action) {
            return index;
        }
    }
    return 0;
}

}  // namespace

AgentActionModel::AgentActionModel() : baseModel_(applyAction) {
}

AgentActionModel::AgentActionModel(ActionModel baseModel)
    : baseModel_(baseModel) {
    // An empty std::function would fail when called, and this
    // project uses no exceptions, so fall back to the standard
    // rule instead of ever calling an empty function.
    if (!baseModel_) {
        baseModel_ = applyAction;
    }
}

void AgentActionModel::restore(Action action) {
    const std::size_t at = static_cast<std::size_t>(actionIndex(action));
    restored_[at] = true;
}

void AgentActionModel::unrestore(Action action) {
    const std::size_t at = static_cast<std::size_t>(actionIndex(action));
    restored_[at] = false;
}

bool AgentActionModel::isRestored(Action action) const {
    const std::size_t at = static_cast<std::size_t>(actionIndex(action));
    return restored_[at];
}

void AgentActionModel::setBase(ActionModel model) {
    // An empty model means applyAction, exactly as in the
    // constructor. The restored marks are kept unchanged.
    if (!model) {
        model = applyAction;
    }
    baseModel_ = model;
}

ActionModel AgentActionModel::current() const {
    // Copy the base model and the marks now (a snapshot), so
    // later restore() or unrestore() calls do not change a
    // model already handed out (plan 3.15 Q2). A caller that
    // asks again after a restoration gets the new marks.
    const ActionModel baseModel = baseModel_;
    const std::array<bool, 4> restored = restored_;
    return [baseModel, restored](const WorldState& state,
                                 Action action) -> ActionResult {
        const std::size_t at = static_cast<std::size_t>(actionIndex(action));
        if (restored[at]) {
            return applyAction(state, action);
        }
        return baseModel(state, action);
    };
}

ActionModel AgentActionModel::base() const {
    return baseModel_;
}

}  // namespace aih
