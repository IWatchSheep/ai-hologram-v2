// Purpose: Builds C1 and its state, runs the decision
// step in its fixed order, the final check, and the counts.
// Introduced in: Stage 3e (rebuild)
// Architecture: The order of decide() is the behaviour
// (plan 3.9): check the committed prediction first, then
// retry a deferred investigation, then finish pending
// work, then the fault activation point (H1), then choose
// and commit. Changing that order would change what the
// budget is spent on and which lines are logged, so the
// reference's order is followed exactly.

#include "agent/single_agent.h"

#include <optional>
#include <utility>
#include <vector>

#include "agent/agent_model.h"
#include "agent/agent_types.h"
#include "agent/observation.h"
#include "agent/single_agent_state.h"
#include "model/world_model.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

SingleAgentState::SingleAgentState(const Observation& first,
                                   AgentSettings settingsValue,
                                   std::optional<WorldState> startingBelief,
                                   std::optional<ActionModel> baseModel,
                                   std::vector<Position> notDead)
    : settings(settingsValue),
      belief(startingBelief.has_value() ? startingBelief.value() : first.state),
      history{first},
      notDeadOverrides(std::move(notDead)),
      agentModel(baseModel.has_value() ? AgentActionModel(baseModel.value())
                                       : AgentActionModel()),
      // The predictor's model asks agentModel.current() for
      // a fresh snapshot at every evaluation, so restoring
      // an action takes effect at once (plan 3.15 Q2). The
      // lambda captures this state, which is why the state
      // cannot be copied (see the header).
      predictor(WorldModel(
                    ActionModel([this](const WorldState& state, Action action) {
                        return agentModel.current()(state, action);
                    })),
                settingsValue.budgetPerStep) {
}

SingleAgent::SingleAgent(const Observation& first, AgentSettings settings,
                         std::optional<WorldState> startingBelief,
                         std::optional<ActionModel> baseModel,
                         std::vector<Position> notDeadOverrides)
    : state(first, settings, startingBelief, baseModel,
            std::move(notDeadOverrides)) {
}

Decision SingleAgent::decide(const Observation& observation) {
    state.decisionStep = state.decisionStep + 1;
    state.predictor.startStep();
    if (state.decisionStep != 1) {
        state.history.push_back(observation);
    }
    if (state.committed.has) {
        checkCommitted(observation);
    }
    // A deferred investigation that is deferred again ends
    // the step with a safe stop. The activation hook is
    // still called before returning: a step that ends here
    // still has its activation point (IE-1 H1).
    const std::optional<Decision> stopped = retryDeferred();
    if (stopped.has_value()) {
        if (state.activationHook) {
            state.activationHook(state.decisionStep);
        }
        return stopped.value();
    }
    continuePendingWork();
    // The fault activation point (IE-1 H1). The hook is a
    // std::function member: empty in every Stage 3 run, so
    // nothing happens here until Stage 4 attaches to it.
    if (state.activationHook) {
        state.activationHook(state.decisionStep);
    }
    return chooseAndCommit();
}

void SingleAgent::finalCheck(const Observation& observation) {
    state.predictor.startStep();
    state.history.push_back(observation);
    if (state.committed.has) {
        checkCommitted(observation);
    }
}

const std::vector<std::string>& SingleAgent::logLines() const {
    return state.log;
}

int SingleAgent::mismatchCount() const {
    return static_cast<int>(state.mismatches.size());
}

int SingleAgent::correctionCount() const {
    return static_cast<int>(state.corrections.size());
}

int SingleAgent::verifiedCount() const {
    int verified = 0;
    for (const CorrectionRecord& record : state.corrections) {
        if (record.status == CorrectionStatus::Verified) {
            verified = verified + 1;
        }
    }
    return verified;
}

int SingleAgent::safeStopCount() const {
    return state.safeStops;
}

int SingleAgent::totalEvaluations() const {
    return state.predictor.usedTotal();
}

}  // namespace aih
