// Purpose: Declares applying one fault to one reasoner:
// the reasoner's parts, and tryApplyFault (plan 3.4).
// Introduced in: Stage 4c (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. Applying a fault changes only the
// reasoner's parts it is handed (its belief, its action
// model, its "not dead" overrides, its predictor cache);
// it never touches the true world and never uses a budget.
// Amendment A3 in plain words: an impossible belief is
// never created, so some faults are refused with a reason.

#ifndef AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_APPLY_H
#define AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_APPLY_H

#include <string>
#include <vector>

#include "agent/agent_model.h"
#include "agent/naive_agent.h"
#include "agent/observation.h"
#include "agent/single_agent.h"
#include "agent/step_predictor.h"
#include "experiment/fault_event.h"
#include "experiment/fault_model.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// The parts of one reasoner that the injector is allowed
// to change, as references to the agent's own members.
// C1 and C0 each have the same four parts, named in their
// own state structs, so one bundle serves both.
struct ReasonerParts {
    WorldState& belief;
    AgentActionModel& actionModel;
    std::vector<Position>& notDeadOverrides;
    StepPredictor& predictor;
};

// Points the parts at the corresponding members of one
// agent's state.
ReasonerParts partsOf(SingleAgentState& state);
ReasonerParts partsOf(NaiveAgentState& state);

// Whether one fault was applied, and the exact reason
// when it was not.
struct ApplyOutcome {
    bool applied = false;
    std::string reason;
};

// The object's position just before its most recent move
// in the true history, or no position when it has not
// moved. F5 reads its previous square from here (the TRUE
// history), never from the reasoner's belief.
bool previousObject(const std::vector<Observation>& trueHistory,
                    Position& previous);

// Tries to apply one event to one reasoner (plan 3.4,
// exactly, in the reference's order of checks). On any
// applied fault the predictor's cache is emptied (D-036),
// because the belief or the model has changed; a refused
// fault changes nothing and empties nothing.
ApplyOutcome tryApplyFault(const FaultEvent& event, ReasonerParts parts,
                           FaultedActions& faultedActions,
                           const std::vector<Observation>& trueHistory);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_APPLY_H
