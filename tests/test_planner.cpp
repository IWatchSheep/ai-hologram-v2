// Purpose: Tests for the A* planner (PL1, PL2a, PL2b, PL3,
// PL8, PL9, PL10, plan Section 6.3).
// Introduced in: Stage 3c (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld. Searches run
// through the shared helpers of agent_test_support (rule R4).
// Expected values come from checks.py (check_planner), and
// each description starts with its check id.

#include <cstddef>
#include <vector>

#include "agent/dead_squares.h"
#include "agent/heuristic.h"
#include "agent/planner.h"
#include "agent_test_support.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/world_state.h"

namespace aih {
namespace {

// True when no discovered state has its object on a dead
// square of map (PL3).
bool noDiscoveredOnDead(const SearchResult& result, const DeadSquareMap& map) {
    for (const DiscoveredState& known : result.discovered) {
        if (readDeadStatus(map, known.state.object)) {
            return false;
        }
    }
    return true;
}

void checkPl1(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const SearchRun run = runMainSearch(demo, 200, 1);
    const std::vector<Action> expected = {Action::East, Action::East,
                                          Action::East, Action::East};
    bool ok = run.result.kind == SearchKind::Found;
    ok = ok && planActions(run.result.plan) == expected;
    ok = ok && run.usedThisStep == 16 && !run.result.plan.empty();
    ok = ok && run.result.plan.back().expected.object == Position{8, 3};
    check(counter, isValidWorld(demo) && ok,
          "PL1: D, B 200, reserve 1: Found, East x4, 16 evaluations; "
          "object on (8, 3)");
}

void checkPl2(TestCounter& counter) {
    const WorldState lp = lpWorld();
    const SearchRun run = runMainSearch(lp, 1000000, 1);
    const std::vector<Action> expected = {
        Action::West,  Action::West, Action::West, Action::North, Action::West,
        Action::North, Action::East, Action::East, Action::East};
    bool ok = run.result.kind == SearchKind::Found;
    ok = ok && planActions(run.result.plan) == expected;
    ok = ok && run.usedThisStep == 60 && run.result.improvedPaths == 1;
    ok = ok && run.result.reopenedStates == 0;
    check(counter, isValidWorld(lp) && ok,
          "PL2a: LP: Found, 9-action plan W W W N W N E E E, 60 "
          "evaluations, 1 improved, 0 reopened");
    const WorldState small = smallWorld();
    const SearchRun smallRun = runMainSearch(small, 200, 1);
    const std::vector<Action> two = {Action::East, Action::East};
    const bool okB = smallRun.result.kind == SearchKind::Found &&
                     planActions(smallRun.result.plan) == two;
    check(counter, isValidWorld(small) && okB,
          "PL2b: SMALL: Found, East, East");
}

void checkPl3(TestCounter& counter) {
    const SearchRun first = runMainSearch(demoWorld(), 200, 1);
    const SearchRun lp = runMainSearch(lpWorld(), 1000000, 1);
    const SearchRun small = runMainSearch(smallWorld(), 200, 1);
    const SearchRun l2 = runMainSearch(l2World(), 1000000, 1);
    WorldState d2 = demoWorld();
    d2 = withWallAt(d2, Position{4, 3}, true);
    d2 = withAgentAt(d2, Position{7, 1});
    const std::vector<Position> targets = {Position{4, 4}, Position{3, 3},
                                           Position{4, 2}};
    const SearchRun walk = runActiveWalk(d2, targets);
    bool ok = noDiscoveredOnDead(first.result, first.map);
    ok = ok && noDiscoveredOnDead(lp.result, lp.map);
    ok = ok && noDiscoveredOnDead(small.result, small.map);
    ok = ok && noDiscoveredOnDead(l2.result, l2.map);
    ok = ok && noDiscoveredOnDead(walk.result, walk.map);
    const bool valid = isValidWorld(demoWorld()) && isValidWorld(d2);
    check(counter, valid && ok,
          "PL3: in PL1, PL2a, PL2b, PL6, PL7 no discovered object is on "
          "a dead square");
}

void checkPl8(TestCounter& counter) {
    bool same = true;
    const WorldState roots[2] = {demoWorld(), lpWorld()};
    for (const WorldState& root : roots) {
        const SearchRun x = runMainSearch(root, 1000000, 1);
        const SearchRun y = runMainSearch(root, 1000000, 1);
        same = same && x.result.kind == y.result.kind;
        same = same && x.usedThisStep == y.usedThisStep;
        same = same && x.result.plan.size() == y.result.plan.size();
        for (std::size_t i = 0; i < x.result.plan.size(); i = i + 1) {
            same = same && x.result.plan[i].action == y.result.plan[i].action;
            same = same && x.result.plan[i].expected.agent ==
                               y.result.plan[i].expected.agent;
            same = same && x.result.plan[i].expected.object ==
                               y.result.plan[i].expected.object;
        }
        same = same && isValidWorld(root);
    }
    check(counter, same,
          "PL8: D and LP searched twice: identical kinds, plans, counts");
}

void checkPl9Pl10(TestCounter& counter) {
    bool match = true;
    const WorldState roots[2] = {demoWorld(), lpWorld()};
    for (const WorldState& root : roots) {
        int calls = 0;
        const SearchRun run = runMainSearchCounting(root, 1000000, 1, calls);
        match = match && run.result.kind == SearchKind::Found;
        match = match && calls == run.usedThisStep;
    }
    check(counter, match,
          "PL9: counting model calls equal charged evaluations (D, LP)");
    const WorldState lg = lgWorld();
    FreeComputationCounts counts;
    const DeadSquareMap map = computeDeadSquares(lg, {}, {}, counts);
    const int value = heuristic(lg, map, counts);
    const SearchRun run = runMainSearch(lg, 200, 1);
    bool ok = value == 4 && run.result.kind == SearchKind::Found;
    ok = ok && run.result.plan.empty() && run.usedThisStep == 0;
    check(counter, isValidWorld(lg) && ok,
          "PL10: LG: heuristic 4; Found with empty plan, 0 evaluations");
}

}  // namespace

void runPlannerTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Planner ---\n";
    }
    checkPl1(counter);
    checkPl2(counter);
    checkPl3(counter);
    checkPl8(counter);
    checkPl9Pl10(counter);
}

}  // namespace aih
