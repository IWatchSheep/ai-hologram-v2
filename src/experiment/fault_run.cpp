// Purpose: Implements the C1 run with faults (plan 3.6):
// the Stage 3 run loop itself, with the true history kept
// through onObservation and the injector on the hook.
// Introduced in: Stage 4c (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. It never copies the run loop: it hands
// runSingleAgent watch-only watchers, so a run with an
// empty event list gives exactly the Stage 3 result, and
// one loop cannot drift from the other. onObservation
// only watches: it appends every observation to the true
// history the injector reads for F5, before the agent
// receives that observation.

#include "experiment/fault_run.h"

#include <functional>
#include <vector>

#include "agent/agent_types.h"
#include "agent/observation.h"
#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "experiment/fault_apply.h"
#include "experiment/fault_event.h"
#include "experiment/fault_injector.h"
#include "world/world_state.h"

namespace aih {

FaultRunResult runWithFaults(const WorldState& trueInitialState,
                             SingleAgent& agent, AgentSettings settings,
                             const std::vector<FaultEvent>& events,
                             FaultRunWatchers watchers) {
    std::vector<Observation> trueHistory;
    FaultInjector injector(events, &trueHistory);
    agent.state.activationHook = [&injector, &agent](int step) {
        injector.atActivationPoint(partsOf(agent.state), step);
    };
    RunWatchers runWatchers;
    runWatchers.onObservation = [&trueHistory](const SingleAgent&,
                                               const Observation& observation) {
        trueHistory.push_back(observation);
    };
    runWatchers.afterDecision = [&watchers, &injector](
                                    const SingleAgent& watched,
                                    const Observation& observation,
                                    const Decision& decision) {
        if (watchers.afterDecision) {
            watchers.afterDecision(watched, observation, decision, injector);
        }
    };
    runWatchers.afterFinalCheck = [&watchers, &injector](
                                      const SingleAgent& watched,
                                      const Observation& observation) {
        if (watchers.afterFinalCheck) {
            watchers.afterFinalCheck(watched, observation, injector);
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
