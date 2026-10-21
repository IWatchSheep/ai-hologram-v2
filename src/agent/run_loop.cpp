// Purpose: Runs C1 in its world until it succeeds, stops
// unresolved, or reaches the world-step limit (plan 3.11).
// Introduced in: Stage 3e (rebuild)
// Architecture: Every observation is built from the
// Environment only. On a safe stop the same state is
// delivered again with no last action; on an action the
// environment steps and the new state is delivered with
// the action and its outcome. The watchers are called
// after the decision and after the final check, and only
// watch: they receive const references and return void,
// so a run with watchers behaves exactly as one without.

#include "agent/run_loop.h"

#include <optional>
#include <string>
#include <vector>

#include "agent/agent_log.h"
#include "agent/agent_types.h"
#include "agent/observation.h"
#include "agent/single_agent.h"
#include "world/action.h"
#include "world/environment.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

// Builds the result of a finished run: the agent's counts
// and its log followed by the four result lines (plan 3.12).
RunResult makeResult(const SingleAgent& agent, const std::string& outcome,
                     int worldSteps) {
    RunResult result;
    result.outcome = outcome;
    result.worldSteps = worldSteps;
    result.safeStops = agent.safeStopCount();
    result.mismatches = agent.mismatchCount();
    result.corrections = agent.correctionCount();
    result.verified = agent.verifiedCount();
    result.evaluations = agent.totalEvaluations();
    result.lines = agent.logLines();
    result.lines.push_back(
        resultLine(outcome, worldSteps, agent.safeStopCount()));
    result.lines.push_back(mismatchesLine(agent.mismatchCount()));
    result.lines.push_back(
        correctionsLine(agent.correctionCount(), agent.verifiedCount()));
    result.lines.push_back(evaluationsLine(agent.totalEvaluations()));
    return result;
}

// The next decision step number the agent will take.
int nextStep(const SingleAgent& agent) {
    return agent.state.decisionStep + 1;
}

// Ends the run when the goal was reached or the step
// limit was hit: the final check runs, the final watcher
// is called, and the result is built. Otherwise no result.
std::optional<RunResult> finishRun(Environment& environment, SingleAgent& agent,
                                   const Observation& next, int worldSteps,
                                   AgentSettings settings,
                                   const RunWatchers& watchers) {
    const bool reached =
        environment.state().object == environment.state().destination;
    if (!reached && worldSteps < settings.maxWorldSteps) {
        return std::nullopt;
    }
    agent.finalCheck(next);
    if (watchers.afterFinalCheck) {
        watchers.afterFinalCheck(agent, next);
    }
    if (reached) {
        return makeResult(agent, "Success", worldSteps);
    }
    return makeResult(agent, "Step limit", worldSteps);
}

}  // namespace

RunResult runSingleAgent(const WorldState& trueInitialState, SingleAgent& agent,
                         AgentSettings settings, RunWatchers watchers) {
    if (trueInitialState.object == trueInitialState.destination) {
        return makeResult(agent, "Success", 0);
    }
    Environment environment(trueInitialState);
    Observation current;
    current.decisionStep = 1;
    current.state = trueInitialState;
    return continueRun(environment, current, 0, agent, settings, watchers);
}

RunResult continueRun(Environment& environment, Observation current,
                      int worldSteps, SingleAgent& agent,
                      AgentSettings settings, RunWatchers watchers) {
    while (true) {
        const Decision decision = agent.decide(current);
        if (watchers.afterDecision) {
            watchers.afterDecision(agent, current, decision);
        }
        if (!decision.isAction) {
            if (agent.safeStopCount() >= settings.safeStopLimit) {
                return makeResult(agent, "Stopped -- unresolved", worldSteps);
            }
            Observation next;
            next.decisionStep = nextStep(agent);
            next.state = current.state;
            current = next;
            continue;
        }
        const ActionResult stepped = environment.step(decision.action);
        worldSteps = worldSteps + 1;
        Observation next;
        next.decisionStep = nextStep(agent);
        next.state = environment.state();
        next.lastAction = decision.action;
        next.lastOutcome = stepped.outcome;
        const std::optional<RunResult> finished =
            finishRun(environment, agent, next, worldSteps, settings, watchers);
        if (finished.has_value()) {
            return finished.value();
        }
        current = next;
    }
}

}  // namespace aih
