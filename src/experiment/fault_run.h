// Purpose: Declares the run with faults for C1 and C0:
// the Stage 3 run loop itself, connected through watchers
// to fault injection (plan 3.6).
// Introduced in: Stage 4c (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. The run with faults reuses the Stage 3
// loop (runSingleAgent) through its watch-only watchers
// instead of copying the loop, so there is only one loop
// and it cannot drift from the fault-free one (D-075).
// The extra watchers here only watch too (rule R6): they
// receive const references and nothing is read back.

#ifndef AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_RUN_H
#define AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_RUN_H

#include <functional>
#include <vector>

#include "agent/agent_types.h"
#include "agent/naive_agent.h"
#include "agent/observation.h"
#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "experiment/fault_event.h"
#include "experiment/fault_injector.h"
#include "world/world_state.h"

namespace aih {

// One run with faults: the RunResult exactly as
// runSingleAgent returned it, how many decision steps the
// agent took, and one activation record per event. Scores
// are added in Stage 4d and observed evaluations in 4e.
struct FaultRunResult {
    RunResult run;
    int decisionSteps = 0;
    std::vector<ActivationRecord> activations;
};

// Optional extra watchers for a C1 run with faults, called
// after the fault run's own watchers. They only watch: the
// agent, the observation (and the decision) and the
// injector, all as const references. The scorer argument
// is added in Stage 4d.
struct FaultRunWatchers {
    std::function<void(const SingleAgent&, const Observation&, const Decision&,
                       const FaultInjector&)>
        afterDecision;
    std::function<void(const SingleAgent&, const Observation&,
                       const FaultInjector&)>
        afterFinalCheck;
};

// The same extra watcher for a C0 run with faults (C0 has
// no final check, so there is no afterFinalCheck).
struct NaiveFaultRunWatchers {
    std::function<void(const NaiveAgent&, const Observation&, const Decision&,
                       const FaultInjector&)>
        afterDecision;
};

// Runs C1 with faults: the Stage 3 loop, with the true
// observation history kept through onObservation, the
// injector connected to the agent's activation hook, and
// the hook emptied again after the loop returns.
FaultRunResult runWithFaults(const WorldState& trueInitialState,
                             SingleAgent& agent, AgentSettings settings,
                             const std::vector<FaultEvent>& events,
                             FaultRunWatchers watchers = {});

// The same for C0, through its Stage 3 loop.
FaultRunResult runWithFaults(const WorldState& trueInitialState,
                             NaiveAgent& agent, AgentSettings settings,
                             const std::vector<FaultEvent>& events,
                             NaiveFaultRunWatchers watchers = {});

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_RUN_H
