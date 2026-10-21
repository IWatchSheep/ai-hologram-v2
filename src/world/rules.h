// Purpose: Declares movement outcomes and the pure
// action-application function.
// Introduced in: Stage 1b (rebuild)
// Architecture: Rules calculate a new world state without
// changing the old one, so later stages can use the same
// function to predict an action and Environment can use it
// to advance the one true state.

#ifndef AI_HOLOGRAM_SRC_WORLD_RULES_H
#define AI_HOLOGRAM_SRC_WORLD_RULES_H

#include <string>

#include "world/action.h"
#include "world/world_state.h"

namespace aih {

// An enum class groups the possible results of one attempted
// move. The names are not plain integers, so an outcome can
// never be mixed up with a count or a coordinate.
enum class MoveOutcome { Moved, Pushed, Blocked, InvalidState };

// Returns the readable name of an outcome, such as "Moved".
std::string outcomeName(MoveOutcome outcome);

// The result of applying one action: a brand new state plus
// what happened. The state that was passed in is unchanged;
// this result holds the next state instead.
struct ActionResult {
    WorldState state{};
    MoveOutcome outcome = MoveOutcome::InvalidState;
};

// Applies one action to current, in the order of plan
// Section 3.4, and returns a new state. The parameter is a
// const reference: it is read but never changed and not
// copied. Returning a new value instead of changing the
// input is what lets later stages predict safely.
ActionResult applyAction(const WorldState& current, Action action);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_WORLD_RULES_H
