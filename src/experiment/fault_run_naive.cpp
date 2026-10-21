// Purpose: Implements the C0 run with faults (plan 3.6):
// the Stage 3 C0 loop itself, with the true history kept
// through onObservation, the injector on the activation
// hook, and the scorer reading the agent at each phase.
// Introduced in: Stage 4c (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. Like the C1 run with faults, it reuses
// the Stage 3 loop through watch-only watchers instead of
// copying it, so one loop cannot drift. C0 has no final
// check, so there is no afterFinalCheck watcher here, and
// its view has no corrections, mismatches or committed
// prediction (quirk Q10). The scorer only reads.

#include "experiment/fault_run.h"

#include <functional>
#include <vector>

#include "agent/agent_types.h"
#include "agent/naive_agent.h"
#include "agent/observation.h"
#include "agent/run_loop.h"
#include "experiment/fault_apply.h"
#include "experiment/fault_event.h"
#include "experiment/fault_injector.h"
#include "experiment/scorer.h"
#include "world/world_state.h"

namespace aih {

FaultRunResult runWithFaults(const WorldState& trueInitialState,
                             NaiveAgent& agent, AgentSettings settings,
                             const std::vector<FaultEvent>& events,
                             NaiveFaultRunWatchers watchers) {
    std::vector<Observation> trueHistory;
    FaultInjector injector(events, &trueHistory);
    Scorer scorer(events, trueInitialState, &trueHistory);
    const ScoredView view = viewOf(agent.state);
    agent.state.activationHook = [&scorer, &view, &injector, &agent](int step) {
        scorer.endPhaseA(view, step);
        injector.atActivationPoint(partsOf(agent.state), step);
        scorer.afterActivation(view, injector.records(), step);
    };
    NaiveRunWatchers runWatchers;
    runWatchers.onObservation = [&trueHistory](const NaiveAgent&,
                                               const Observation& observation) {
        trueHistory.push_back(observation);
    };
    runWatchers.afterDecision = [&watchers, &scorer, &view, &injector](
                                    const NaiveAgent& watched,
                                    const Observation& observation,
                                    const Decision& decision) {
        scorer.afterDecide(view, decision);
        if (watchers.afterDecision) {
            watchers.afterDecision(watched, observation, decision, injector,
                                   scorer);
        }
    };
    FaultRunResult result;
    result.run = runSingleAgent(trueInitialState, agent, settings, runWatchers);
    // The hook is emptied, so the agent never points at an
    // injector that is about to be destroyed.
    agent.state.activationHook = std::function<void(int)>{};
    scorer.finish(view);
    result.decisionSteps = agent.state.decisionStep;
    result.activations = injector.records();
    result.eventScores = scorer.eventScores();
    result.score = scorer.runScore();
    return result;
}

}  // namespace aih
