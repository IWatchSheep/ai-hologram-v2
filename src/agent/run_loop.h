// Purpose: Declares the run loop for C1: its result, its
// watch-only watchers, and its two entry points (plan 3.11).
// Introduced in: Stage 3e (rebuild)
// Architecture: The loop owns the Environment and builds
// every observation from it. The watchers only watch
// (rule R6): each is a std::function taking const
// references and returning void, an empty one is not
// called, and nothing in the run reads a watcher back -
// "the chain can be replaced by the camera".

#ifndef AI_HOLOGRAM_SRC_AGENT_RUN_LOOP_H
#define AI_HOLOGRAM_SRC_AGENT_RUN_LOOP_H

#include <functional>
#include <string>
#include <vector>

#include "agent/agent_types.h"
#include "agent/naive_agent.h"
#include "agent/observation.h"
#include "agent/single_agent.h"
#include "world/environment.h"
#include "world/world_state.h"

namespace aih {

// How one run ended, and its counted totals. lines is C1's
// log followed by the four result lines of plan 3.12.
struct RunResult {
    std::string outcome;
    int worldSteps = 0;
    int safeStops = 0;
    int mismatches = 0;
    int corrections = 0;
    int verified = 0;
    int evaluations = 0;
    std::vector<std::string> lines;
};

// Watch-only watchers for a C1 run (plan 3.11, D-071).
// afterDecision is called with the observation that was
// just decided on and the decision, afterFinalCheck with
// the final observation, both after the agent has acted.
struct RunWatchers {
    std::function<void(const SingleAgent&, const Observation&, const Decision&)>
        afterDecision;
    std::function<void(const SingleAgent&, const Observation&)> afterFinalCheck;
};

// Runs C1 from a true initial state. When the object is
// already on the destination the run succeeds at once and
// decide() is never called (plan 3.11).
RunResult runSingleAgent(const WorldState& trueInitialState, SingleAgent& agent,
                         AgentSettings settings, RunWatchers watchers = {});

// Continues a run already under way, from the given
// environment, observation and world-step count. Tests
// call this directly to start after a seeded action
// (plan 3.14), and runSingleAgent calls it for a new run.
RunResult continueRun(Environment& environment, Observation current,
                      int worldSteps, SingleAgent& agent,
                      AgentSettings settings, RunWatchers watchers);

// Watch-only watcher for a C0 run (plan 3.11, D-071). It
// only watches (rule R6): a std::function taking const
// references and returning void, an empty one is not
// called, and nothing in the run reads it back.
struct NaiveRunWatchers {
    std::function<void(const NaiveAgent&, const Observation&, const Decision&)>
        afterDecision;
};

// Runs C0 from a true initial state (plan 3.11). The same
// loop as C1's, but C0 has no final check, never safe-stops
// and writes no log lines: its lines are only the four
// result lines, with zero mismatches and corrections.
RunResult runSingleAgent(const WorldState& trueInitialState, NaiveAgent& agent,
                         AgentSettings settings,
                         NaiveRunWatchers watchers = {});

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_RUN_LOOP_H
