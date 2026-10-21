// Purpose: Implements the grid movement and pushing rules
// of plan Section 3.4, in the original's exact order.
// Introduced in: Stage 1b (rebuild)
// Architecture: This pure rules component serves both
// predictions and Environment. It takes a const WorldState&
// and returns a new state, so calling it never changes the
// world that was passed in.

#include "world/rules.h"

#include <string>

#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

std::string outcomeName(MoveOutcome outcome) {
    // A switch selects the name for whichever enum value was
    // provided. Every case returns, so no case falls through.
    switch (outcome) {
        case MoveOutcome::Moved: {
            return "Moved";
        }
        case MoveOutcome::Pushed: {
            return "Pushed";
        }
        case MoveOutcome::Blocked: {
            return "Blocked";
        }
        case MoveOutcome::InvalidState: {
            return "InvalidState";
        }
    }
    return "Unknown";
}

ActionResult applyAction(const WorldState& current, Action action) {
    // Rule 0: an invalid state never changes.
    if (!isValidWorld(current)) {
        return {current, MoveOutcome::InvalidState};
    }
    const Position target = moved(current.agent, action);
    // Rule 1: a wall in the target cell blocks the move.
    if (isWall(current, target)) {
        return {current, MoveOutcome::Blocked};
    }
    // Work on a copy, so the input state stays untouched.
    WorldState next = current;
    next.agent = target;
    // Rule 2: no object in the target cell means a plain move.
    if (target != current.object) {
        return {next, MoveOutcome::Moved};
    }
    const Position beyond = moved(current.object, action);
    // Rule 3: a wall beyond the object blocks the push, and
    // the copy is thrown away, so nothing has changed.
    if (isWall(current, beyond)) {
        return {current, MoveOutcome::Blocked};
    }
    // Rule 4: the object moves beyond and the agent follows.
    next.object = beyond;
    return {next, MoveOutcome::Pushed};
}

}  // namespace aih
