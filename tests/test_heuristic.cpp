// Purpose: Tests for the dead-square cache and the heuristic
// (DS6, DS7, DS8a, DS8b, DS8c, DS8d, plan Section 6.2).
// Introduced in: Stage 3b (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld. Expected
// values come from checks.py, and each description starts
// with its check id.

#include <vector>

#include "agent/dead_square_cache.h"
#include "agent/dead_squares.h"
#include "agent/heuristic.h"
#include "agent_test_support.h"
#include "model/budget_counter.h"
#include "model/world_model.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

void checkDs6(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState moved = withAgentAt(demo, Position{2, 3});
    const WorldState walled = withWallAt(demo, Position{2, 7}, true);
    FreeComputationCounts counts;
    DeadSquareCache cache;
    std::vector<int> runs;
    cache.get(demo, {}, {}, counts);
    runs.push_back(counts.analysisRuns);
    cache.get(demo, {}, {}, counts);
    runs.push_back(counts.analysisRuns);
    cache.get(moved, {}, {}, counts);
    runs.push_back(counts.analysisRuns);
    cache.get(walled, {}, {}, counts);
    runs.push_back(counts.analysisRuns);
    cache.get(walled, {Position{1, 1}}, {}, counts);
    runs.push_back(counts.analysisRuns);
    cache.get(walled, {Position{1, 1}}, {}, counts);
    runs.push_back(counts.analysisRuns);
    const std::vector<int> expected = {1, 1, 1, 2, 3, 3};
    const bool valid =
        isValidWorld(demo) && isValidWorld(moved) && isValidWorld(walled);
    check(counter, valid && runs == expected,
          "DS6: cache analysis runs after six gets are 1, 1, 1, 2, 3, 3");
}

void checkDs7(TestCounter& counter) {
    const WorldState demo = demoWorld();
    FreeComputationCounts counts;
    const DeadSquareMap map = computeDeadSquares(demo, {}, {}, counts);
    int modelCalls = 0;
    const ActionModel countingModel = [&modelCalls](const WorldState& state,
                                                    Action action) {
        modelCalls = modelCalls + 1;
        return applyAction(state, action);
    };
    const WorldModel model(countingModel);
    BudgetCounter budget(10);
    const bool local = localCheckSaysNotDead(demo, map, Position{5, 3}, counts);
    const int value = heuristic(demo, map, counts);
    const bool untouched = modelCalls == 0 && budget.used() == 0;
    const bool passed = isValidWorld(demo) && local && value == 4 && untouched;
    check(counter, passed,
          "DS7: local check and heuristic need no model and no budget; "
          "true and 4");
}

void checkDs8(TestCounter& counter) {
    const WorldState demo = demoWorld();
    FreeComputationCounts counts;
    const DeadSquareMap map = computeDeadSquares(demo, {}, {}, counts);
    const WorldState nearer = withAgentAt(demo, Position{4, 3});
    const int first = heuristic(demo, map, counts);
    const int second = heuristic(nearer, map, counts);
    const bool validA = isValidWorld(demo) && isValidWorld(nearer);
    check(counter, validA && first == 4 && second == 3,
          "DS8a: heuristic on D is 4; agent at (4, 3) gives 3");
    const WorldState stuck = withObjectAt(demo, Position{1, 1});
    const int infinite = heuristic(stuck, map, counts);
    check(counter, isValidWorld(stuck) && infinite == kInfiniteHeuristic,
          "DS8b: object on (1, 1) gives kInfiniteHeuristic");
    FreeComputationCounts overCounts;
    const DeadSquareMap over =
        computeDeadSquares(demo, {Position{1, 1}}, {}, overCounts);
    WorldState far = withAgentAt(demo, Position{5, 5});
    far = withObjectAt(far, Position{1, 1});
    const int fallback = heuristic(far, over, overCounts);
    check(counter, isValidWorld(far) && fallback == 16,
          "DS8c: overridden (1, 1), agent (5, 5): 16 (9 + 7 fallback)");
    const bool counted =
        counts.heuristicCalls == 3 && overCounts.heuristicCalls == 1;
    check(counter, counted, "DS8d: heuristicCalls counts each call (3 and 1)");
}

}  // namespace

void runHeuristicTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Heuristic ---\n";
    }
    checkDs6(counter);
    checkDs7(counter);
    checkDs8(counter);
}

}  // namespace aih
