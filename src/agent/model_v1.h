// Purpose: Declares model verification V1: replaying the
// recorded uses of one action with the standard rule.
// Introduced in: Stage 3d (rebuild)
// Architecture: The work is a plain record (the action and
// the history indices still to replay), so it can wait over
// several decision steps when the budget runs out (plan 3.8).

#ifndef AI_HOLOGRAM_SRC_AGENT_MODEL_V1_H
#define AI_HOLOGRAM_SRC_AGENT_MODEL_V1_H

#include <vector>

#include "agent/observation.h"
#include "agent/step_predictor.h"
#include "world/action.h"

namespace aih {

// The model V1 work still to do: the action under suspicion
// and the indices k into the history whose recorded use of
// that action has not yet been replayed, in order.
struct ModelV1Work {
    Action action = Action::North;
    std::vector<int> pendingIndices;
};

// How one call of continueModelV1 ended.
enum class V1Progress { Pending, Failed, Done };

// The work for action a: the indices k, in order, whose
// recorded observation says a was the last action taken.
ModelV1Work startModelV1(Action action,
                         const std::vector<Observation>& history);

// Works through the pending indices while any remain
// (plan 3.8). Each replay is a variant prediction: it uses
// the standard rule (applyAction), not the agent's current
// model, because the question is whether the standard rule
// reproduces the record; and it is charged like any variant.
V1Progress continueModelV1(ModelV1Work& work,
                           const std::vector<Observation>& history,
                           StepPredictor& predictor, int reserve);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_MODEL_V1_H
