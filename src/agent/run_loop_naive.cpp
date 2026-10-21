// Purpose: Runs C0 in its world until it succeeds or
// reaches the world-step limit (plan 3.11).
// Introduced in: Stage 3f (rebuild)
// Architecture: The same loop as C1's run loop, with the
// parts C0 does not have left out: C0 never safe-stops, so
// no observation is ever redelivered, and it has no final
// check, so a run ends as soon as the goal is reached or
// the step limit is hit. Every observation is built from
// the Environment only. The watcher is called after each
// decision and only watches: it receives const references
// and returns void, so a watched run behaves exactly as an
// unwatched one.

#include "agent/run_loop.h"

#include <string>

#include "agent/agent_log.h"
#include "agent/agent_types.h"
#include "agent/naive_agent.h"
#include "agent/observation.h"
#include "world/environment.h"
#include "world/world_state.h"

namespace aih {
namespace {

// Builds C0's result: only the four result lines, with
// safe stops, mismatches and corrections all zero, because
// C0 makes no checks and records nothing (Phase B 11.8).
RunResult makeNaiveResult(const NaiveAgent& agent, const std::string& outcome,
                          int worldSteps) {
    RunResult result;
    result.outcome = outcome;
    result.worldSteps = worldSteps;
    result.evaluations = agent.totalEvaluations();
    result.lines.push_back(resultLine(outcome, worldSteps, 0));
    result.lines.push_back(mismatchesLine(0));
    result.lines.push_back(correctionsLine(0, 0));
    result.lines.push_back(evaluationsLine(agent.totalEvaluations()));
    return result;
}

}  // namespace

RunResult runSingleAgent(const WorldState& trueInitialState, NaiveAgent& agent,
                         AgentSettings settings, NaiveRunWatchers watchers) {
    if (trueInitialState.object == trueInitialState.destination) {
        return makeNaiveResult(agent, "Success", 0);
    }
    Environment environment(trueInitialState);
    Observation current;
    current.decisionStep = 1;
    current.state = trueInitialState;
    int worldSteps = 0;
    while (true) {
        const Decision decision = agent.decide(current);
        if (watchers.afterDecision) {
            watchers.afterDecision(agent, current, decision);
        }
        const ActionResult stepped = environment.step(decision.action);
        worldSteps = worldSteps + 1;
        Observation next;
        next.decisionStep = agent.state.decisionStep + 1;
        next.state = environment.state();
        next.lastAction = decision.action;
        next.lastOutcome = stepped.outcome;
        if (environment.state().object == environment.state().destination) {
            return makeNaiveResult(agent, "Success", worldSteps);
        }
        if (worldSteps >= settings.maxWorldSteps) {
            return makeNaiveResult(agent, "Step limit", worldSteps);
        }
        current = next;
    }
}

}  // namespace aih
