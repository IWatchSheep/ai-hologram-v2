// Purpose: Implements model verification V1 (plan 3.8).
// Introduced in: Stage 3d (rebuild)
// Architecture: Each recorded use of the suspected action is
// replayed from the state recorded before it, with the
// standard rule, and compared with the state and outcome
// recorded after it. The replay is a variant prediction, so
// it is charged to the step budget and never cached.

#include "agent/model_v1.h"

#include <cstddef>
#include <optional>
#include <vector>

#include "agent/observation.h"
#include "agent/step_predictor.h"
#include "model/prediction.h"
#include "world/action.h"
#include "world/rules.h"

namespace aih {

ModelV1Work startModelV1(Action action,
                         const std::vector<Observation>& history) {
    ModelV1Work work;
    work.action = action;
    for (std::size_t k = 0; k < history.size(); k = k + 1) {
        if (history[k].lastAction.has_value()) {
            if (history[k].lastAction.value() == action) {
                work.pendingIndices.push_back(static_cast<int>(k));
            }
        }
    }
    return work;
}

V1Progress continueModelV1(ModelV1Work& work,
                           const std::vector<Observation>& history,
                           StepPredictor& predictor, int reserve) {
    while (!work.pendingIndices.empty()) {
        if (predictor.remaining() <= reserve) {
            return V1Progress::Pending;
        }
        const int k = work.pendingIndices.front();
        if (k < 1 || k >= static_cast<int>(history.size())) {
            return V1Progress::Failed;
        }
        const std::size_t index = static_cast<std::size_t>(k);
        const Observation& recorded = history[index];
        const std::optional<Prediction> replay = predictor.predictVariant(
            history[index - 1].state, work.action, applyAction);
        if (!replay.has_value()) {
            return V1Progress::Pending;
        }
        if (replay->state != recorded.state) {
            return V1Progress::Failed;
        }
        if (!recorded.lastOutcome.has_value()) {
            return V1Progress::Failed;
        }
        if (replay->outcome != recorded.lastOutcome.value()) {
            return V1Progress::Failed;
        }
        work.pendingIndices.erase(work.pendingIndices.begin());
    }
    return V1Progress::Done;
}

}  // namespace aih
