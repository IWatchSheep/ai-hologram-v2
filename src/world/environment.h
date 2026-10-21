// Purpose: Declares the environment that owns the one true
// world state.
// Introduced in: Stage 1b (rebuild)
// Architecture: Actions change the true state only through
// Environment::step, which applies the shared rules. Nothing
// else holds a mutable copy of the true world.

#ifndef AI_HOLOGRAM_SRC_WORLD_ENVIRONMENT_H
#define AI_HOLOGRAM_SRC_WORLD_ENVIRONMENT_H

#include "world/action.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {

// A class groups data with the operations that control it.
// Unlike a struct, its members are private by default, and
// the private section below protects the true state from
// being changed except through step().
class Environment {
public:
    // A constructor creates an Environment holding a copy
    // of the supplied initial world, so later changes to
    // the caller's state cannot affect this one. explicit
    // prevents a WorldState being turned into an Environment
    // by accident.
    explicit Environment(const WorldState& initialState);

    // Applies one action with the shared rules, stores the
    // resulting state and adds one to the step count, even
    // when the outcome is InvalidState.
    ActionResult step(Action action);

    // A const member function cannot change the Environment
    // it is called on. Returning a const reference also
    // prevents callers from changing the state it exposes.
    const WorldState& state() const;

    // const here too: asking for the count changes nothing.
    int stepCount() const;

private:
    WorldState currentState_;
    int stepCount_ = 0;
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_WORLD_ENVIRONMENT_H
