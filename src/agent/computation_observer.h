// Purpose: Declares the watch-only observer for the IE-1
// hooks H3 and H4: reports of charged evaluations, dead-status
// reads and heuristic values.
// Introduced in: Stage 3b (rebuild)
// Architecture: An observer only watches (rule R6): every
// function it holds takes const references, copies or a const
// pointer and returns void, so nothing an agent decides can
// depend on an observer, and an agent never reads one back.
// "The chain can be replaced by the camera": a chain could
// pull, a camera cannot. An empty function is not called.

#ifndef AI_HOLOGRAM_SRC_AGENT_COMPUTATION_OBSERVER_H
#define AI_HOLOGRAM_SRC_AGENT_COMPUTATION_OBSERVER_H

#include <functional>

#include "model/prediction.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// The three report functions, each held in a std::function
// member as a hook. In Stage 3b only the H4 reports (dead
// reads and heuristic values) are sent; onEvaluation is used
// by the step predictor in Stage 3c. Nothing is attached in
// normal runs, so every hook is empty and never called.
struct ComputationObserver {
    std::function<void(const WorldState&, Action, const ActionModel*,
                       const Prediction&)>
        onEvaluation;
    std::function<void(Position, bool)> onDeadRead;
    std::function<void(const WorldState&, int)> onHeuristic;
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_COMPUTATION_OBSERVER_H
