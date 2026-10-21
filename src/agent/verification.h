// Purpose: Declares trap verification V1 and V2, the
// prediction-difference test, the active-test targets and
// the alternative prediction (plan 3.8).
// Introduced in: Stage 3d (rebuild)
// Architecture: trapV1Check, trapV2Test, predictionsDiffer
// and activeTestTargets live in verification.cpp, and
// alternativePrediction in alternative_prediction.cpp (the
// planned split, plan Section 4).

#ifndef AI_HOLOGRAM_SRC_AGENT_VERIFICATION_H
#define AI_HOLOGRAM_SRC_AGENT_VERIFICATION_H

#include <optional>
#include <utility>
#include <vector>

#include "agent/dead_squares.h"
#include "agent/observation.h"
#include "agent/records.h"
#include "agent/step_predictor.h"
#include "model/prediction.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// The V1 trap test (protocol 11.5): the local check does not
// say "not dead" at p, and the recorded history shows the
// object never went on from p to the destination.
bool trapV1Check(const WorldState& belief, const DeadSquareMap& map, Position p,
                 const std::vector<Observation>& history,
                 FreeComputationCounts& counts);

// How a trap V2 test ended: the claim is Verified, Rejected
// (a push escapes), or Waiting for budget.
enum class TrapV2Result { Verified, Rejected, Waiting };

// The V2 trap test (plan 3.8): from every cell the agent
// could stand on to push the object at p, predict the push
// in a hypothetical state. The hypothetical states are
// predicted because the agent is not really standing there:
// each prediction is a variant with the current model, so
// it is charged and never cached. anomalies counts the
// hypothetical states that were not valid worlds.
TrapV2Result trapV2Test(const WorldState& belief, const DeadSquareMap& map,
                        Position p, StepPredictor& predictor,
                        const ActionModel& currentModel, int reserve,
                        int& anomalies);

// True when two predictions differ in outcome, agent
// position or object position (plan 3.8).
bool predictionsDiffer(const Prediction& first, const Prediction& second);

// The cells an active test could walk to, with the move that
// then tests cell: for each direction, the cell the agent
// would stand on, when it is not a believed wall and not the
// believed object.
std::vector<std::pair<Position, Action>> activeTestTargets(
    const WorldState& belief, Position cell);

// The alternative prediction for testing a correction on a
// later step (plan 3.8): what the action would predict if
// the correction were undone. For a model correction that is
// a variant with the old model on the belief itself; for a
// wall or object correction, a variant with the current
// model on the belief with the old value put back.
std::optional<Prediction> alternativePrediction(
    const CorrectionRecord& correction, const WorldState& belief, Action action,
    StepPredictor& predictor, const ActionModel& currentModel,
    const ActionModel& oldModel);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_VERIFICATION_H
