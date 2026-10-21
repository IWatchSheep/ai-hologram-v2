// Purpose: Declares the run with faults for C1 and C0:
// the Stage 3 run loop itself, connected through watchers
// to fault injection and to the scorer (plan 3.6).
// Introduced in: Stage 4c (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. The run with faults reuses the Stage 3
// loop (runSingleAgent) through its watch-only watchers
// instead of copying the loop, so there is only one loop
// and it cannot drift from the fault-free one (D-075).
// The scorer only reads the agent (rule R6); the extra
// watchers here only watch too: they receive const
// references, including the scorer, and nothing is read
// back from them.

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
#include "experiment/scorer.h"
#include "experiment/scorer_types.h"
#include "world/world_state.h"

namespace aih {

// One run with faults: the RunResult exactly as
// runSingleAgent returned it, how many decision steps the
// agent took, one activation record per event, the scores
// (Stage 4d), and how many evaluations the watch-only
// observer saw (Stage 4e; 0 when observe is false).
struct FaultRunResult {
    RunResult run;
    int decisionSteps = 0;
    std::vector<ActivationRecord> activations;
    std::vector<EventScore> eventScores;
    RunScore score;
    int observedEvaluations = 0;
};

// Optional extra watchers for a C1 run with faults, called
// after the scorer's own watchers. They only watch: the
// agent, the observation (and the decision), the injector
// and the scorer, all as const references.
struct FaultRunWatchers {
    std::function<void(const SingleAgent&, const Observation&, const Decision&,
                       const FaultInjector&, const Scorer&)>
        afterDecision;
    std::function<void(const SingleAgent&, const Observation&,
                       const FaultInjector&, const Scorer&)>
        afterFinalCheck;
};

// The same extra watcher for a C0 run with faults (C0 has
// no final check, so there is no afterFinalCheck).
struct NaiveFaultRunWatchers {
    std::function<void(const NaiveAgent&, const Observation&, const Decision&,
                       const FaultInjector&, const Scorer&)>
        afterDecision;
};

// Runs C1 with faults: the Stage 3 loop, with the true
// observation history kept through onObservation, the
// injector and the scorer connected to the agent's
// activation hook, and the hook emptied again after the
// loop returns. With observe true (the default), a
// watch-only ComputationObserver that only calls the scorer
// is attached to the predictor and the dead-square cache
// (IE-1 H3, H4); with observe false nothing is attached and
// observedEvaluations stays 0. Primary scoring never
// depends on the observer: it only sets opportunity steps.
FaultRunResult runWithFaults(const WorldState& trueInitialState,
                             SingleAgent& agent, AgentSettings settings,
                             const std::vector<FaultEvent>& events,
                             bool observe = true,
                             FaultRunWatchers watchers = {});

// The same for C0, through its Stage 3 loop.
FaultRunResult runWithFaults(const WorldState& trueInitialState,
                             NaiveAgent& agent, AgentSettings settings,
                             const std::vector<FaultEvent>& events,
                             bool observe = true,
                             NaiveFaultRunWatchers watchers = {});

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_RUN_H
