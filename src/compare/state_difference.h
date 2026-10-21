// Purpose: Declares the results of comparing a predicted
// world state with an observed one, and a whole prediction
// with an observed action result.
// Introduced in: Stage 2b (rebuild)
// Architecture: compareStates fills a StateDifference,
// comparePrediction wraps it with the two outcomes, and
// hasDifferences, outcomesDiffer and matches answer the
// simple questions later stages ask about those results.

#ifndef AI_HOLOGRAM_SRC_COMPARE_STATE_DIFFERENCE_H
#define AI_HOLOGRAM_SRC_COMPARE_STATE_DIFFERENCE_H

#include <vector>

#include "model/prediction.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {

// How one position differs: whether it differs at all, and
// the predicted and observed positions. Members are written
// one per line, as the style rules require.
struct PositionDifference {
    bool differs;
    Position predicted;
    Position observed;
};

// How one wall cell differs: the cell, and whether the wall
// was blocked in the predicted and in the observed state.
struct WallDifference {
    Position cell;
    bool predictedBlocked;
    bool observedBlocked;
};

// Every difference between two states, in a fixed order:
// agent, object, destination, then the differing wall
// cells in row-by-row order (see compareStates).
struct StateDifference {
    PositionDifference agent;
    PositionDifference object;
    PositionDifference destination;
    std::vector<WallDifference> walls;
};

// Returns true when any position differs or the wall list
// is not empty, so a caller need not inspect every member.
bool hasDifferences(const StateDifference& difference);

// Compares every part of predicted with observed, in the
// order agent, object, destination, then walls row by row.
// It reads walls only with its own loop counters, never
// with a stored position, so it is safe for any input.
StateDifference compareStates(const WorldState& predicted,
                              const WorldState& observed);

// How a whole prediction differs from what was observed:
// the two outcomes, plus the state difference.
struct PredictionComparison {
    MoveOutcome predictedOutcome;
    MoveOutcome observedOutcome;
    StateDifference state;
};

// Returns true when the predicted and observed outcomes
// are not the same outcome.
bool outcomesDiffer(const PredictionComparison& comparison);

// Returns true only when the outcomes are equal and there
// are no state differences: fitting the outcome alone is
// not enough for a prediction to count as matching.
bool matches(const PredictionComparison& comparison);

// Builds a PredictionComparison from a prediction and the
// observed result of the same action.
PredictionComparison comparePrediction(const Prediction& prediction,
                                       const ActionResult& observed);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_COMPARE_STATE_DIFFERENCE_H
