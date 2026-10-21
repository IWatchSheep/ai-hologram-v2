// Purpose: Implements the true world's state and its
// attempted-action counter.
// Introduced in: Stage 1b (rebuild)
// Architecture: Environment applies the shared movement
// rules to its private state, so the true world and every
// prediction follow exactly the same rules.

#include "world/environment.h"

#include "world/action.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {

Environment::Environment(const WorldState& initialState)
    // The member initializer list copies the supplied world
    // into this Environment's own private state.
    : currentState_(initialState), stepCount_(0) {
}

ActionResult Environment::step(Action action) {
    const ActionResult result = applyAction(currentState_, action);
    currentState_ = result.state;
    // Every attempted action counts, blocked ones included.
    stepCount_ = stepCount_ + 1;
    return result;
}

const WorldState& Environment::state() const {
    return currentState_;
}

int Environment::stepCount() const {
    return stepCount_;
}

}  // namespace aih
