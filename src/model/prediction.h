// Purpose: Declares the expected result of one action, as
// predicted from a belief before the action is taken.
// Introduced in: Stage 2a (rebuild)
// Architecture: WorldModel returns a Prediction, which later
// stages compare with the observed result of the action.

#ifndef AI_HOLOGRAM_SRC_MODEL_PREDICTION_H
#define AI_HOLOGRAM_SRC_MODEL_PREDICTION_H

#include "world/action.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {

// One prediction: the action that was asked about, the
// outcome the model expects, and the state the model
// expects to see afterwards. Members are written one per
// line, as the style rules require.
struct Prediction {
    Action action;
    MoveOutcome outcome;
    WorldState state;
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_MODEL_PREDICTION_H
