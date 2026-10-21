// Purpose: Implements the F3 faulty action model (plan
// 3.4): a push predicted in a rotated direction.
// Introduced in: Stage 4c (rebuild)
// Architecture: makeFaultyModel copies the faulted-actions
// table at the moment it is called, exactly as the
// reference does. The standard rule (applyAction) still
// decides every non-push and every unfaulted action.

#include "experiment/fault_model.h"

#include <cstddef>

#include "experiment/fault_event.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {

Action rotatedAction(Action action, Rotation rotation) {
    // The four actions in index order, turned left or right.
    const std::array<Action, 4> left = {Action::West, Action::North,
                                        Action::East, Action::South};
    const std::array<Action, 4> right = {Action::East, Action::South,
                                         Action::West, Action::North};
    const std::size_t index = static_cast<std::size_t>(actionIndex(action));
    if (rotation == Rotation::Left) {
        return left[index];
    }
    return right[index];
}

ActionModel makeFaultyModel(FaultedActions table) {
    // The table is copied into the model now, so a later F3
    // fault (which rebuilds the model) cannot change a model
    // that was already handed out.
    return [table](const WorldState& state, Action action) -> ActionResult {
        const ActionResult standard = applyAction(state, action);
        const std::size_t index = static_cast<std::size_t>(actionIndex(action));
        if (!table.faulted[index]) {
            return standard;
        }
        if (standard.outcome != MoveOutcome::Pushed) {
            return standard;
        }
        // The agent moves onto the object's old cell; the
        // object is predicted one cell further in the rotated
        // direction, not in the action's own direction.
        const Position agentCell = moved(state.agent, action);
        const Action turned = rotatedAction(action, table.rotation[index]);
        const Position objectCell = moved(agentCell, turned);
        if (isWall(state, objectCell)) {
            return ActionResult{state, MoveOutcome::Blocked};
        }
        WorldState next = state;
        next.agent = agentCell;
        next.object = objectCell;
        return ActionResult{next, MoveOutcome::Pushed};
    };
}

}  // namespace aih
