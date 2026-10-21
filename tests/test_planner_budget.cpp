// Purpose: Tests for the planner's budget, reserve and
// fallback behaviour (PL4a, PL4b, PL5a, PL5b, PL6, PL7, plan
// Section 6.3).
// Introduced in: Stage 3c (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld. Searches run
// through the shared helpers of agent_test_support (rule R4).
// Expected values come from checks.py (check_planner), and
// each description starts with its check id.

#include <vector>

#include "agent/planner.h"
#include "agent_test_support.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

// D2: the demo world with a wall added at (4, 3).
WorldState d2World() {
    return withWallAt(demoWorld(), Position{4, 3}, true);
}

void checkPl4(TestCounter& counter) {
    const WorldState d2 = d2World();
    const SearchRun a = runMainSearch(d2, 10, 1);
    bool ok = a.result.kind == SearchKind::OutOfBudget;
    ok = ok && a.usedThisStep == 9;
    ok = ok && a.remaining == 1;
    check(counter, isValidWorld(d2) && ok,
          "PL4a: D2, B 10, reserve 1: OutOfBudget, 9 used, 1 remaining");
    const SearchRun b = runMainSearch(d2, 10, 3);
    bool okB = b.result.kind == SearchKind::OutOfBudget;
    okB = okB && b.usedThisStep == 7 && b.remaining == 3;
    check(counter, okB,
          "PL4b: D2, B 10, reserve 3: OutOfBudget, 7 used, 3 remaining");
}

void checkPl5(TestCounter& counter) {
    const WorldState d2 = d2World();
    const SearchRun a = runMainSearch(d2, 10, 1);
    const SearchRun b = runMainSearch(d2, 10, 3);
    const std::vector<Action> expected = {Action::North, Action::East};
    bool ok = planActions(fallback(a.result)) == expected;
    ok = ok && planActions(fallback(b.result)) == expected;
    check(counter, isValidWorld(d2) && ok,
          "PL5a: fallback of both PL4 searches is North, East");
    const SearchRun tiny = runMainSearch(d2, 1, 1);
    bool okB = tiny.result.kind == SearchKind::OutOfBudget;
    okB = okB && tiny.result.discovered.size() == 1;
    okB = okB && fallback(tiny.result).empty();
    check(counter, okB,
          "PL5b: D2, B 1, reserve 1: only root discovered; fallback empty");
}

void checkPl6(TestCounter& counter) {
    const WorldState l2 = l2World();
    const SearchRun run = runMainSearch(l2, 1000000, 1);
    bool ok = run.result.kind == SearchKind::Exhausted;
    ok = ok && run.usedThisStep == 156;
    ok = ok && run.result.discovered.size() == 39;
    check(counter, isValidWorld(l2) && ok,
          "PL6: L2, B 1000000: Exhausted, 156 evaluations, 39 states");
}

void checkPl7(TestCounter& counter) {
    WorldState root = d2World();
    root = withAgentAt(root, Position{7, 1});
    const std::vector<Position> targets = {Position{4, 4}, Position{3, 3},
                                           Position{4, 2}};
    const SearchRun run = runActiveWalk(root, targets);
    const std::vector<Action> expected = {Action::South, Action::West,
                                          Action::West, Action::West};
    bool ok = run.result.kind == SearchKind::Found;
    ok = ok && planActions(run.result.plan) == expected;
    ok = ok && !run.result.plan.empty();
    ok = ok && run.result.plan.back().expected.agent == Position{4, 2};
    ok = ok && run.usedThisStep == 16;
    check(counter, isValidWorld(root) && ok,
          "PL7: walk to (4, 4), (3, 3) or (4, 2): South, West, West, "
          "West to (4, 2), 16 evaluations");
}

}  // namespace

void runPlannerBudgetTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Planner budget ---\n";
    }
    checkPl4(counter);
    checkPl5(counter);
    checkPl6(counter);
    checkPl7(counter);
}

}  // namespace aih
