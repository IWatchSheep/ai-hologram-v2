// Purpose: Implements the state and prediction comparison
// of plan Section 3.4, reporting every difference.
// Introduced in: Stage 2b (rebuild)
// Architecture: compareStates is the one place walls are
// compared (rule R4). It never changes either state; it
// only builds a StateDifference describing them.

#include "compare/state_difference.h"

#include <cstddef>

#include "model/prediction.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

// Builds the difference record for one position.
PositionDifference comparePosition(const Position& predicted,
                                   const Position& observed) {
    return PositionDifference{predicted != observed, predicted, observed};
}

}  // namespace

bool hasDifferences(const StateDifference& difference) {
    if (difference.agent.differs) {
        return true;
    }
    if (difference.object.differs) {
        return true;
    }
    if (difference.destination.differs) {
        return true;
    }
    return !difference.walls.empty();
}

StateDifference compareStates(const WorldState& predicted,
                              const WorldState& observed) {
    StateDifference difference;
    difference.agent = comparePosition(predicted.agent, observed.agent);
    difference.object = comparePosition(predicted.object, observed.object);
    difference.destination =
        comparePosition(predicted.destination, observed.destination);
    // Walls are read only with these loop counters, which
    // always run from 0 to gridSize - 1. A stored position
    // is never used as a wall index, because a position in
    // an invalid state could lie outside the grid and read
    // outside the array; the loop counters cannot.
    for (int y = 0; y < gridSize; y = y + 1) {
        for (int x = 0; x < gridSize; x = x + 1) {
            const std::size_t row = static_cast<std::size_t>(y);
            const std::size_t column = static_cast<std::size_t>(x);
            const bool predictedBlocked = predicted.walls[row][column];
            const bool observedBlocked = observed.walls[row][column];
            if (predictedBlocked != observedBlocked) {
                const WallDifference wall{Position{x, y}, predictedBlocked,
                                          observedBlocked};
                difference.walls.push_back(wall);
            }
        }
    }
    return difference;
}

bool outcomesDiffer(const PredictionComparison& comparison) {
    return comparison.predictedOutcome != comparison.observedOutcome;
}

bool matches(const PredictionComparison& comparison) {
    if (outcomesDiffer(comparison)) {
        return false;
    }
    return !hasDifferences(comparison.state);
}

PredictionComparison comparePrediction(const Prediction& prediction,
                                       const ActionResult& observed) {
    return PredictionComparison{
        prediction.outcome, observed.outcome,
        compareStates(prediction.state, observed.state)};
}

}  // namespace aih
