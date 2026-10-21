// Purpose: Declares the agent tests' shared inputs: the
// observations and histories, the hand-written test
// models, and the seeding of a C1 state (plan 4, v1.3).
// Introduced in: Stage 3e (rebuild)
// Architecture: One copy of each helper lives here (rule
// R4); the five helpers that two test files each wrote for
// themselves in 3c and 3d move here in 3e. agent_test_support
// is full, so these helpers get their own file.

#ifndef AI_HOLOGRAM_TESTS_AGENT_TEST_FIXTURES_H
#define AI_HOLOGRAM_TESTS_AGENT_TEST_FIXTURES_H

#include <algorithm>
#include <vector>

#include "agent/observation.h"
#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {

// One observation with only its step and state set: no last
// action and no last outcome, as at the start of a run.
Observation makeObservation(int step, const WorldState& state);

// The two observations of one action on the true world:
// before it, and after it with the action and its outcome.
std::vector<Observation> twoObservations(const WorldState& trueBefore,
                                         Action action);

// The 4-entry East history of checks.py: the demo world,
// then three East actions, each recorded.
std::vector<Observation> eastHistory();

// The Watch of checks.py: records what the run loop's
// watchers see (delivered and final observations, belief
// checks, committed actions, charges). watchers() hands
// out the two watch-only functions (rule R6); tests only
// read the records afterwards.
struct StepWatch {
    std::vector<Observation> delivered;
    std::vector<Observation> finals;
    bool beliefsTrue = true;
    int checks = 0;
    bool committed = true;
    int maxUsed = 0;

    RunWatchers watchers() {
        RunWatchers watchers;
        watchers.afterDecision = [this](const SingleAgent& watched,
                                        const Observation& given,
                                        const Decision& decision) {
            delivered.push_back(given);
            if (watched.state.decisionStep > 1) {
                beliefsTrue =
                    beliefsTrue && watched.state.belief == given.state;
                checks = checks + 1;
            }
            if (decision.isAction && !watched.state.committed.has) {
                committed = false;
            }
            maxUsed = std::max(maxUsed, watched.state.predictor.usedThisStep());
        };
        watchers.afterFinalCheck = [this](const SingleAgent& watched,
                                          const Observation& given) {
            finals.push_back(given);
            beliefsTrue = beliefsTrue && watched.state.belief == given.state;
            checks = checks + 1;
        };
        return watchers;
    }
};

// A model that pushes the object North instead of East.
ActionResult pushNorth(const WorldState& state, Action action);

// A model that always blocks, leaving the state unchanged.
ActionResult alwaysBlocked(const WorldState& state, Action action);

// A model under which East always "moves" one cell.
ActionResult alwaysMovedEast(const WorldState& state, Action action);

// Seeds wall corrections with ids 1, 2, ... in order, all
// Consistent and untested at step 0, and sets
// nextCorrectionId to the count + 1 (plan 3.14, seed wall).
// The belief already shows the new values; nothing is applied.
void seedWallCorrections(SingleAgent& agent, const std::vector<Position>& cells,
                         bool newBlocked, bool oldBlocked);

// Seeds model V1 work for East with its history, from the
// demo world (plan 3.14, seed v1): count recorded East
// steps are appended, pending indices 1 to count.
void seedModelV1(SingleAgent& agent, int count);

// Seeds a plan in place: one step, the given action with
// the belief as its expected state, kind Goal (seed plan).
void seedPlan(SingleAgent& agent, Action action);

// Seeds a committed action at step 1 (plan 3.14, seed
// committed): decisionStep 1, the action predicted from
// the belief with the standard rule, no tested corrections.
void seedCommitted(SingleAgent& agent, Action action);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_AGENT_TEST_FIXTURES_H
