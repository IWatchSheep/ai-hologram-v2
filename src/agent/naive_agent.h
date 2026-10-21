// Purpose: Declares C0, the naive single agent: its state and
// its one decision step (plan 3.10, protocol Phase B 11.8).
// Introduced in: Stage 3f (rebuild)
// Architecture: NaiveAgent owns one NaiveAgentState (the public
// member state) and works only on it, exactly as SingleAgent
// does. C0 exists only as a reference floor for H5: it plans
// from its own belief with C1's planner, heuristic, dead-square
// analysis and budgeted StepPredictor, but it never checks a
// prediction against an observation, so a wrong belief is
// never noticed. The state's predictor model is a lambda that
// calls agentModel.current() at every evaluation, so the state
// (and the agent) cannot be copied.

#ifndef AI_HOLOGRAM_SRC_AGENT_NAIVE_AGENT_H
#define AI_HOLOGRAM_SRC_AGENT_NAIVE_AGENT_H

#include <functional>
#include <optional>
#include <vector>

#include "agent/agent_model.h"
#include "agent/agent_types.h"
#include "agent/dead_square_cache.h"
#include "agent/dead_squares.h"
#include "agent/observation.h"
#include "agent/planner.h"
#include "agent/step_predictor.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// Everything C0 remembers between decision steps (plan 3.10).
// All members are plain and public, so tests and (in 3g) the
// trace writer can read them (D-019). The override lists stay
// empty: C0 has no corrections, so nothing ever fills them.
struct NaiveAgentState {
    NaiveAgentState(const Observation& first, AgentSettings settings,
                    std::optional<WorldState> startingBelief,
                    std::optional<ActionModel> baseModel);

    // The predictor's model lambda captures this state (it
    // calls agentModel.current() through it), so copying the
    // state would leave the copy predicting through the
    // original's model. Copying is deleted, not just unused.
    NaiveAgentState(const NaiveAgentState&) = delete;
    NaiveAgentState& operator=(const NaiveAgentState&) = delete;

    AgentSettings settings;
    WorldState belief{};
    DeadSquareCache deadCache{};
    FreeComputationCounts freeCounts{};
    std::vector<Position> notDeadOverrides;
    std::vector<Position> deadOverrides;
    AgentActionModel agentModel;
    StepPredictor predictor;
    std::vector<PlanStep> plan;
    int decisionStep = 0;
    int searches = 0;
    // The fault activation point (IE-1 H1): a std::function
    // member used as a hook. It is empty in every Stage 3
    // run; Stage 4 will attach the fault injector to it, and
    // nothing ever reads anything back from it.
    std::function<void(int)> activationHook;
};

// The naive single agent (plan 3.10). It always returns an
// action: it makes no checks, investigations, corrections,
// claims, log lines or safe stops.
class NaiveAgent {
public:
    // The belief is the starting belief when one is given,
    // and the first observation's state otherwise.
    NaiveAgent(const Observation& first, AgentSettings settings,
               std::optional<WorldState> startingBelief = std::nullopt,
               std::optional<ActionModel> baseModel = std::nullopt);

    NaiveAgent(const NaiveAgent&) = delete;
    NaiveAgent& operator=(const NaiveAgent&) = delete;

    // One decision step (plan 3.10). The observation is
    // never read: C0 ignores every observation after the
    // first (protocol Phase B 11.8), because never checking
    // predictions against what is observed is exactly what
    // makes C0 the naive reference agent.
    Decision decide(const Observation& observation);

    // Evaluations charged over all steps so far.
    int totalEvaluations() const;
    // How many times the planner's search has been run.
    int searchCount() const;

    // The whole state, public so tests and the trace writer
    // can read it (plan 3.10, D-019).
    NaiveAgentState state;

private:
    // Plans from the belief when the plan is used up: the
    // search (reserve 0), then the fallback, then the first
    // moving action or North (plan 3.10, step 3).
    void planFromBelief();
    // The first action (North, East, South, West) predicted
    // Moved or Pushed from the belief, with its predicted
    // state; or North with the unchanged belief when no
    // action is, or the budget cannot pay for one.
    PlanStep firstMovingOrNorth();
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_NAIVE_AGENT_H
