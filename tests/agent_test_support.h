// Purpose: Declares the shared test worlds and helpers for
// the agent tests: the demo world, its edited copies, the
// named layouts of plan Section 6, and a line comparison.
// Introduced in: Stage 3a (rebuild)
// Architecture: Every agent test gets its worlds from here,
// so there is one copy of each (rule R4). Later sub-stages
// add seeding helpers to this file.

#ifndef AI_HOLOGRAM_TESTS_AGENT_TEST_SUPPORT_H
#define AI_HOLOGRAM_TESTS_AGENT_TEST_SUPPORT_H

#include <ostream>
#include <string>
#include <vector>

#include "agent/dead_squares.h"
#include "agent/planner.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// The demo world, parsed from demoLayout() (rule R4).
WorldState demoWorld();

// The named layouts of plan Section 6, as parsed worlds.
WorldState lpWorld();
WorldState l2World();
WorldState ltWorld();
WorldState lgWorld();
WorldState ltaWorld();
WorldState boxedWorld();
WorldState smallWorld();

// Returns a copy of world with only the agent moved.
WorldState withAgentAt(const WorldState& world, Position agent);

// Returns a copy of world with only the object moved.
WorldState withObjectAt(const WorldState& world, Position object);

// Returns a copy of world with the wall at cell changed.
// The caller passes interior cells, so the index is safe.
WorldState withWallAt(const WorldState& world, Position cell, bool blocked);

// True when the two line lists are exactly equal. When they
// differ, the first differing line is written to out, so a
// failing check shows where the text first went wrong.
bool linesEqual(const std::vector<std::string>& expected,
                const std::vector<std::string>& actual, std::ostream& out);

// The result of one shared planner search for the Stage 3c
// tests: the search result, the evaluations charged in the
// step, and the dead-square map the search used.
struct SearchRun {
    SearchResult result;
    int usedThisStep = 0;
    int remaining = 0;
    DeadSquareMap map{};
};

// The main goal search of checks.py: goal "object on
// destination", the heuristic of plan 3.3, a fresh predictor
// with the given budget. Added in Stage 3c because both
// planner test files use it (rule R4).
SearchRun runMainSearch(const WorldState& root, int budget, int reserve);

// The same main search, with a counting model whose calls are
// added to modelCalls (the model's own calls and the charged
// evaluations must be equal, PL9).
SearchRun runMainSearchCounting(const WorldState& root, int budget, int reserve,
                                int& modelCalls);

// The walk search of checks.py active_walk: goal "the agent is
// on a target cell", heuristic the smallest manhattan to a
// target (kInfiniteHeuristic when the object is dead), reserve
// 1 and a large budget, unless a budget is given.
SearchRun runActiveWalk(const WorldState& root,
                        const std::vector<Position>& targets);

// The actions of a plan, in order.
std::vector<Action> planActions(const std::vector<PlanStep>& plan);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_AGENT_TEST_SUPPORT_H
