// Purpose: Implements the C0 run with faults (plan 3.6):
// the Stage 3 C0 loop itself, with the true history kept
// through onObservation and the injector on the hook.
// Introduced in: Stage 4c (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. Like the C1 run with faults, it reuses
// the Stage 3 loop through watch-only watchers instead of
// copying it, so one loop cannot drift. C0 has no final
// check, so there is no afterFinalCheck watcher here.

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
#include "world/world_state.h"

namespace aih {

FaultRunResult runWithFaults(const WorldState& trueInitialState,
                             NaiveAgent& agent, AgentSettings settings,
                             const std::vector<FaultEvent>& events,
                             NaiveFaultRunWatchers watchers) {
    std::vector<Observation> trueHistory;
    FaultInjector injector(events, &trueHistory);
    agent.state.activationHook = [&injector, &agent](int step) {
        injector.atActivationPoint(partsOf(agent.state), step);
    };
    NaiveRunWatchers runWatchers;
    runWatchers.onObservation = [&trueHistory](const NaiveAgent&,
                                               const Observation& observation) {
        trueHistory.push_back(observation);
    };
    runWatchers.afterDecision = [&watchers, &injector](
                                    const NaiveAgent& watched,
                                    const Observation& observation,
                                    const Decision& decision) {
        if (watchers.afterDecision) {
            watchers.afterDecision(watched, observation, decision, injector);
        }
    };
    FaultRunResult result;
    result.run = runSingleAgent(trueInitialState, agent, settings, runWatchers);
    // The hook is emptied, so the agent never points at an
    // injector that is about to be destroyed.
    agent.state.activationHook = std::function<void(int)>{};
    result.decisionSteps = agent.state.decisionStep;
    result.activations = injector.records();
    return result;
}

}  // namespace aih
