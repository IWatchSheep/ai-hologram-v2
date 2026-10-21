// Purpose: Builds C0 and runs its decision step: plan only
// when the plan is used up, then take the next planned action
// and set the belief to its stored expected state (plan 3.10).
// Introduced in: Stage 3f (rebuild)
// Architecture: C0 uses the same StepPredictor (limit B, the
// settings' budget per step), planner (search with reserve 0,
// then fallback), main heuristic and dead-square analysis as
// C1, and every prediction goes through the StepPredictor.
// Executing a plan step makes no new prediction: the plan
// already stores the expected state, and reusing it costs
// nothing because C0's belief is, by definition, that stored
// state (protocol Phase B 11.8).

#include "agent/naive_agent.h"

#include <optional>
#include <utility>

#include "agent/agent_model.h"
#include "agent/agent_types.h"
#include "agent/directions.h"
#include "agent/heuristic.h"
#include "agent/observation.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/world_state.h"

namespace aih {

NaiveAgentState::NaiveAgentState(const Observation& first,
                                 AgentSettings settingsValue,
                                 std::optional<WorldState> startingBelief,
                                 std::optional<ActionModel> baseModel)
    : settings(settingsValue),
      belief(startingBelief.has_value() ? startingBelief.value() : first.state),
      agentModel(baseModel.has_value() ? AgentActionModel(baseModel.value())
                                       : AgentActionModel()),
      // The predictor's model asks agentModel.current() for
      // a fresh snapshot at every evaluation (plan 3.15 Q2).
      // The lambda captures this state, which is why the
      // state cannot be copied (see the header).
      predictor(WorldModel(
                    ActionModel([this](const WorldState& state, Action action) {
                        return agentModel.current()(state, action);
                    })),
                settingsValue.budgetPerStep) {
}

NaiveAgent::NaiveAgent(const Observation& first, AgentSettings settings,
                       std::optional<WorldState> startingBelief,
                       std::optional<ActionModel> baseModel)
    : state(first, settings, startingBelief, baseModel) {
}

PlanStep NaiveAgent::firstMovingOrNorth() {
    for (const Action action : kActionOrder) {
        const std::optional<Prediction> predicted =
            state.predictor.predict(state.belief, action);
        if (predicted.has_value()) {
            if (predicted.value().outcome == MoveOutcome::Moved) {
                return PlanStep{action, predicted.value().state};
            }
            if (predicted.value().outcome == MoveOutcome::Pushed) {
                return PlanStep{action, predicted.value().state};
            }
        }
    }
    // No action moves or pushes (every one is Blocked, or the
    // budget could not pay): North with the belief unchanged.
    return PlanStep{Action::North, state.belief};
}

void NaiveAgent::planFromBelief() {
    const DeadSquareMap& map =
        state.deadCache.get(state.belief, state.notDeadOverrides,
                            state.deadOverrides, state.freeCounts);
    auto goal = [](const WorldState& state) {
        return state.object == state.destination;
    };
    auto estimate = [this, &map](const WorldState& candidate) {
        return heuristic(candidate, map, state.freeCounts);
    };
    const SearchResult result =
        search(state.belief, map, goal, estimate, state.predictor, 0);
    state.searches = state.searches + 1;
    if (result.kind == SearchKind::Found) {
        if (!result.plan.empty()) {
            state.plan = result.plan;
            return;
        }
    }
    // An empty Found plan (the belief already shows the goal)
    // counts as "no plan", like an empty fallback.
    state.plan = fallback(result);
    if (state.plan.empty()) {
        state.plan.push_back(firstMovingOrNorth());
    }
}

Decision NaiveAgent::decide(const Observation&) {
    // C0 never reads an observation after the first (protocol
    // Phase B 11.8): later observations would only matter if
    // predictions were checked against them, and checking is
    // exactly what C0 does not do. The parameter is therefore
    // deliberately left unnamed and unused.
    state.decisionStep = state.decisionStep + 1;
    state.predictor.startStep();
    // The fault activation point (IE-1 H1). C0 never checks,
    // so its activation point is at the start of the step.
    // The hook is a std::function member: empty in every
    // Stage 3 run, so nothing happens here until Stage 4.
    if (state.activationHook) {
        state.activationHook(state.decisionStep);
    }
    // C0 plans only at the first decision and whenever its
    // plan is used up; replanning is never triggered by
    // anything observed.
    if (state.plan.empty()) {
        planFromBelief();
    }
    // Executing: take the next plan action and set the belief
    // to that action's stored expected state, with no new
    // prediction (Phase B 11.8).
    const PlanStep step = state.plan.front();
    state.plan.erase(state.plan.begin());
    state.belief = step.expected;
    Decision decision;
    decision.isAction = true;
    decision.action = step.action;
    return decision;
}

int NaiveAgent::totalEvaluations() const {
    return state.predictor.usedTotal();
}

int NaiveAgent::searchCount() const {
    return state.searches;
}

}  // namespace aih
