// Purpose: Declares everything C1 remembers between
// decision steps: its whole state (plan 3.9).
// Introduced in: Stage 3e (rebuild)
// Architecture: All members are plain and public, so tests
// and (in 3g) the trace writer can read and set them
// (D-019). SingleAgent works on this one state. The state's
// predictor model is a lambda that calls
// agentModel.current() at every evaluation, so a restored
// action takes effect at once (plan 3.15 Q2); because that
// lambda captures this state, the state cannot be copied,
// so its copy operations are deleted below.

#ifndef AI_HOLOGRAM_SRC_AGENT_SINGLE_AGENT_STATE_H
#define AI_HOLOGRAM_SRC_AGENT_SINGLE_AGENT_STATE_H

#include <functional>
#include <optional>
#include <string>
#include <vector>

#include "agent/agent_model.h"
#include "agent/agent_types.h"
#include "agent/dead_square_cache.h"
#include "agent/dead_squares.h"
#include "agent/model_v1.h"
#include "agent/observation.h"
#include "agent/planner.h"
#include "agent/records.h"
#include "agent/step_predictor.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// The one prediction C1 has committed to and not yet
// checked: the prediction itself, a copy of the belief it
// was made from, the step it was made at, and the ids of
// the corrections it tests (the V2 bookkeeping of plan 3.9).
struct CommittedPrediction {
    bool has = false;
    Prediction prediction{Action::North, MoveOutcome::Blocked, WorldState{}};
    WorldState beliefBefore{};
    int decisionStep = 0;
    std::vector<int> testedCorrectionIds;
};

// An investigation that was deferred for budget: the belief
// before the action, the action, and a copy of the history
// it must be retried with (plan 3.9).
struct DeferredInvestigation {
    bool has = false;
    WorldState beliefBefore{};
    Action action = Action::North;
    std::vector<Observation> history;
};

// A trap V2 test that is waiting for budget: which
// correction it tests, and the cell it claims is dead.
struct WaitingTrapTest {
    bool has = false;
    int correctionId = 0;
    Position cell{0, 0};
};

// Everything C1 remembers (plan 3.9). The constructor is
// defined in single_agent.cpp with the agent's own.
struct SingleAgentState {
    SingleAgentState(const Observation& first, AgentSettings settings,
                     std::optional<WorldState> startingBelief,
                     std::optional<ActionModel> baseModel,
                     std::vector<Position> notDead);

    // The predictor's model lambda captures this state (it
    // calls agentModel.current() through it), so copying the
    // state would leave the copy predicting through the
    // original's model. Copying is deleted, not just unused.
    SingleAgentState(const SingleAgentState&) = delete;
    SingleAgentState& operator=(const SingleAgentState&) = delete;

    AgentSettings settings;
    WorldState belief{};
    std::vector<Observation> history;
    DeadSquareCache deadCache{};
    FreeComputationCounts freeCounts{};
    std::vector<Position> notDeadOverrides;
    std::vector<Position> deadOverrides;
    AgentActionModel agentModel;
    StepPredictor predictor;
    std::vector<PlanStep> plan;
    PlanKind planKind = PlanKind::None;
    CommittedPrediction committed{};
    std::vector<CorrectionRecord> corrections;
    std::vector<MismatchRecord> mismatches;
    std::optional<ModelV1Work> pendingV1;
    DeferredInvestigation deferred{};
    WaitingTrapTest waitingTrap{};
    int safeStops = 0;
    int decisionStep = 0;
    int nextCorrectionId = 1;
    int trapAnomalies = 0;
    std::vector<std::string> log;
    // The fault activation point (IE-1 H1): a std::function
    // member used as a hook. It is empty in every Stage 3
    // run, so it is never called then; Stage 4 will attach
    // the fault injector to it. Nothing reads it back.
    std::function<void(int)> activationHook;
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_SINGLE_AGENT_STATE_H
