// Purpose: Implements the footprint rule of plan 3.2.
// Introduced in: Stage 3a (rebuild)
// Architecture: The footprint is computed from the belief
// before the action, the action and what was observed.
// C1 does not re-check its whole belief every step: an
// action can only change the cells it reaches, so checking
// those cells is enough to catch a wrong prediction.

#include "agent/footprint.h"

#include <cstddef>
#include <vector>

#include "agent/observation.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {

std::vector<Position> computeFootprint(const WorldState& beliefBefore,
                                       Action action,
                                       const WorldState& observedAfter,
                                       MoveOutcome observedOutcome) {
    const Position c0 = beliefBefore.agent;
    const Position c1 = moved(c0, action);
    const Position c2 = moved(c1, action);
    std::vector<Position> cells;
    cells.push_back(c0);
    cells.push_back(c1);
    // Rule (iii) is observedAfter.object == c1: the object
    // seen one cell ahead means the push cell c2 matters too.
    const bool includeC2 = beliefBefore.object == c1 ||
                           observedOutcome == MoveOutcome::Pushed ||
                           observedAfter.object == c1;
    if (includeC2) {
        cells.push_back(c2);
    }
    return cells;
}

FootprintComparison compareFootprint(const WorldState& predictedState,
                                     MoveOutcome predictedOutcome,
                                     const WorldState& observedState,
                                     MoveOutcome observedOutcome,
                                     const std::vector<Position>& cells) {
    FootprintComparison comparison;
    comparison.outcomesDiffer = predictedOutcome != observedOutcome;
    for (const Position cell : cells) {
        FootprintCellDifference difference;
        difference.cell = cell;
        difference.predictedWall = isWall(predictedState, cell);
        difference.observedWall = isWall(observedState, cell);
        difference.wallDiffers =
            difference.predictedWall != difference.observedWall;
        difference.predictedAgentHere = predictedState.agent == cell;
        difference.observedAgentHere = observedState.agent == cell;
        difference.agentDiffers =
            difference.predictedAgentHere != difference.observedAgentHere;
        difference.predictedObjectHere = predictedState.object == cell;
        difference.observedObjectHere = observedState.object == cell;
        difference.objectDiffers =
            difference.predictedObjectHere != difference.observedObjectHere;
        if (difference.wallDiffers || difference.agentDiffers ||
            difference.objectDiffers) {
            comparison.cells.push_back(difference);
        }
    }
    return comparison;
}

std::vector<Position> computeFootprintForTwoPredictions(
    const WorldState& firstBelief, MoveOutcome firstOutcome,
    const WorldState& secondBelief, MoveOutcome secondOutcome, Action action) {
    const Position c0 = firstBelief.agent;
    const Position c1 = moved(c0, action);
    const Position c2 = moved(c1, action);
    std::vector<Position> cells;
    cells.push_back(c0);
    cells.push_back(c1);
    const bool includeC2 = firstBelief.object == c1 ||
                           secondBelief.object == c1 ||
                           firstOutcome == MoveOutcome::Pushed ||
                           secondOutcome == MoveOutcome::Pushed;
    if (includeC2) {
        cells.push_back(c2);
    }
    return cells;
}

void applyFootprintUpdate(WorldState& belief,
                          const std::vector<Position>& cells,
                          const WorldState& observedState) {
    for (const Position cell : cells) {
        if (!isInsideGrid(cell)) {
            continue;
        }
        // The cell is inside, so isWall reads the stored wall.
        const bool observedWall = isWall(observedState, cell);
        const std::size_t x = static_cast<std::size_t>(cell.x);
        const std::size_t y = static_cast<std::size_t>(cell.y);
        belief.walls[y][x] = observedWall;
        if (observedState.agent == cell) {
            belief.agent = observedState.agent;
        }
        if (observedState.object == cell || belief.object == cell) {
            belief.object = observedState.object;
        }
    }
}

MoveOutcome observedOutcome(const Observation& observation) {
    if (observation.lastOutcome.has_value()) {
        return observation.lastOutcome.value();
    }
    return MoveOutcome::Blocked;
}

std::vector<Position> footprintAfter(const WorldState& beliefBefore,
                                     Action action, const Observation& after) {
    return computeFootprint(beliefBefore, action, after.state,
                            observedOutcome(after));
}

bool isFootprintMatch(const FootprintComparison& comparison) {
    return !comparison.outcomesDiffer && comparison.cells.empty();
}

}  // namespace aih
