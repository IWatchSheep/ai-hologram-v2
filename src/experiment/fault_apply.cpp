// Purpose: Implements tryApplyFault: how each of the five
// fault types is applied to one reasoner, with A3's checks
// (plan 3.4).
// Introduced in: Stage 4c (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. It changes only the reasoner parts it
// is handed: never the true world, and never any budget
// (applying a fault is free). After any applied fault the
// predictor's cache is emptied (D-036); a refused fault
// changes nothing, so its cache is left alone.

#include "experiment/fault_apply.h"

#include <cstddef>
#include <string>
#include <vector>

#include "experiment/fault_event.h"
#include "experiment/fault_model.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

ApplyOutcome notApplied(const std::string& reason) {
    return ApplyOutcome{false, reason};
}

ApplyOutcome appliedFault(ReasonerParts parts) {
    // D-036: the belief or the model has changed, so cached
    // predictions may no longer be what the reasoner would
    // predict now; the cache is emptied. The dead-square
    // cache needs no call: it recomputes by itself when the
    // walls or the overrides change.
    parts.predictor.clearCache();
    return ApplyOutcome{true, ""};
}

ApplyOutcome applyF1(const FaultEvent& event, ReasonerParts parts) {
    if (!event.hasCell) {
        return notApplied("no cell of the required kind");
    }
    if (event.direction == F1Direction::FreeToBlocked) {
        // A3: a wall can never be put under the agent or the
        // object the reasoner believes are there, because
        // such a belief would be impossible.
        if (event.cell == parts.belief.agent) {
            return notApplied("the wall would be under the believed agent");
        }
        if (event.cell == parts.belief.object) {
            return notApplied("the wall would be under the believed object");
        }
    }
    if (isInsideGrid(event.cell)) {
        const std::size_t row = static_cast<std::size_t>(event.cell.y);
        const std::size_t column = static_cast<std::size_t>(event.cell.x);
        parts.belief.walls[row][column] = !parts.belief.walls[row][column];
    }
    return appliedFault(parts);
}

ApplyOutcome applyF2(const FaultEvent& event, ReasonerParts parts) {
    for (const Action direction : event.order) {
        const Position cell = moved(parts.belief.object, direction);
        if (isWall(parts.belief, cell)) {
            continue;
        }
        if (cell == parts.belief.agent) {
            continue;
        }
        parts.belief.object = cell;
        return appliedFault(parts);
    }
    return notApplied("no direction passes the test");
}

ApplyOutcome applyF3(const FaultEvent& event, ReasonerParts parts,
                     FaultedActions& faultedActions) {
    // F3 is never refused. Faults combine: this action's
    // entry is set in the table, the base model is rebuilt
    // from the whole table, and any earlier "restored" mark
    // on this action is removed (IE-1 H2).
    const std::size_t index =
        static_cast<std::size_t>(actionIndex(event.action));
    faultedActions.faulted[index] = true;
    faultedActions.rotation[index] = event.rotation;
    parts.actionModel.setBase(makeFaultyModel(faultedActions));
    parts.actionModel.unrestore(event.action);
    return appliedFault(parts);
}

ApplyOutcome applyF4(const FaultEvent& event, ReasonerParts parts) {
    if (!event.hasCell) {
        return notApplied("the layout has no true dead square");
    }
    parts.notDeadOverrides.push_back(event.cell);
    return appliedFault(parts);
}

ApplyOutcome applyF5(ReasonerParts parts,
                     const std::vector<Observation>& trueHistory) {
    Position previous{0, 0};
    if (!previousObject(trueHistory, previous)) {
        return notApplied("the object has not moved yet");
    }
    // A3: both checks use this reasoner's own belief, while
    // the previous square itself comes from the true history.
    if (previous == parts.belief.agent) {
        return notApplied("the previous square is the believed agent square");
    }
    if (isWall(parts.belief, previous)) {
        return notApplied("the previous square is believed to be a wall");
    }
    parts.belief.object = previous;
    return appliedFault(parts);
}

}  // namespace

ReasonerParts partsOf(SingleAgentState& state) {
    return ReasonerParts{state.belief, state.agentModel, state.notDeadOverrides,
                         state.predictor};
}

ReasonerParts partsOf(NaiveAgentState& state) {
    return ReasonerParts{state.belief, state.agentModel, state.notDeadOverrides,
                         state.predictor};
}

bool previousObject(const std::vector<Observation>& trueHistory,
                    Position& previous) {
    // The earlier object of the last pair of consecutive
    // observations whose object positions differ.
    if (trueHistory.size() < 2) {
        return false;
    }
    for (std::size_t i = trueHistory.size() - 1; i >= 1; i = i - 1) {
        if (trueHistory[i].state.object != trueHistory[i - 1].state.object) {
            previous = trueHistory[i - 1].state.object;
            return true;
        }
    }
    return false;
}

ApplyOutcome tryApplyFault(const FaultEvent& event, ReasonerParts parts,
                           FaultedActions& faultedActions,
                           const std::vector<Observation>& trueHistory) {
    if (event.type == FaultType::F1) {
        return applyF1(event, parts);
    }
    if (event.type == FaultType::F2) {
        return applyF2(event, parts);
    }
    if (event.type == FaultType::F3) {
        return applyF3(event, parts, faultedActions);
    }
    if (event.type == FaultType::F4) {
        return applyF4(event, parts);
    }
    return applyF5(parts, trueHistory);
}

}  // namespace aih
