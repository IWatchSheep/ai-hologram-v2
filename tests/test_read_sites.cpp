// Purpose: Tests for the dead-status read sites reported
// to the observer: the heuristic, planner pruning and the
// cautious choice (SH6; plan Section 6.4).
// Introduced in: Stage 4e (rebuild)
// Architecture: Expected values are copied from checks.py
// check_read_sites. Every agent read of a dead status goes
// through readDeadStatus (IE-1 H4), so an attached observer
// sees exactly these reads, and every reported value equals
// the map's stored value. The observer only watches: it
// records the reports and changes nothing.

#include <cstddef>
#include <optional>
#include <vector>

#include "agent/dead_squares.h"
#include "agent/heuristic.h"
#include "agent/planner.h"
#include "agent/single_agent.h"
#include "agent/step_predictor.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
#include "model/world_model.h"
#include "recording_observer.h"
#include "stage4_suites.h"
#include "test_support.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

// The planner test's estimate: always 0, as in checks.py,
// so the search is driven by the goal alone.
int zeroEstimate(const WorldState&) {
    return 0;
}

// True when every recorded read is inside the grid and its
// value equals the map's stored dead status for that cell.
bool valuesMatch(const RecordingObserver& recorder, const DeadSquareMap& map) {
    bool ok = recorder.cells.size() == recorder.values.size();
    for (std::size_t i = 0; ok && i < recorder.cells.size(); i = i + 1) {
        const Position cell = recorder.cells[i];
        if (!isInsideGrid(cell)) {
            ok = false;
        }
        if (ok) {
            const std::size_t row = static_cast<std::size_t>(cell.y);
            const std::size_t column = static_cast<std::size_t>(cell.x);
            ok = map.dead[row][column] == recorder.values[i];
        }
    }
    return ok;
}

void checkHeuristicSite(TestCounter& counter) {
    const WorldState demo = demoWorld();
    check(counter, isValidWorld(demo), "SH6.1a: D valid");
    FreeComputationCounts counts;
    DeadSquareMap map = computeDeadSquares(demo, {}, {}, counts);
    RecordingObserver recorder;
    ComputationObserver observer = recorder.observer();
    map.observer = &observer;
    const int value = heuristic(demo, map, counts);
    bool ok = value == 4;
    ok = ok && recorder.cells == std::vector<Position>{Position{5, 3}};
    ok = ok && recorder.values == std::vector<bool>{false};
    ok = ok && recorder.heuristics == std::vector<int>{4};
    ok = ok && valuesMatch(recorder, map);
    check(counter, ok, "SH6.1b: heuristic reads (5, 3) false and reports 4");
}

void checkPlannerSite(TestCounter& counter) {
    const WorldState demo = demoWorld();
    FreeComputationCounts counts;
    DeadSquareMap map = computeDeadSquares(demo, {}, {}, counts);
    RecordingObserver recorder;
    ComputationObserver observer = recorder.observer();
    map.observer = &observer;
    StepPredictor predictor(WorldModel(), 200);
    predictor.startStep();
    auto goal = [](const WorldState& state) {
        return state.object == state.destination;
    };
    const SearchResult result =
        search(demo, map, goal, zeroEstimate, predictor, 1);
    bool ok = result.kind == SearchKind::Found;
    ok = ok && !recorder.cells.empty() && recorder.heuristics.empty();
    ok = ok && valuesMatch(recorder, map);
    check(counter, ok,
          "SH6.2: planner pruning reads dead statuses, no heuristic");
}

void checkCautiousSite(TestCounter& counter) {
    const WorldState demo = demoWorld();
    RecordingObserver recorder;
    ComputationObserver observer = recorder.observer();
    SingleAgent agent(makeObservation(1, demo), AgentSettings{});
    agent.state.predictor.startStep();
    agent.state.deadCache.observer = &observer;
    const std::optional<PlanStep> pick = agent.chooseCautiousAction();
    agent.state.deadCache.observer = nullptr;
    bool ok = pick.has_value();
    bool seen = false;
    for (std::size_t i = 0; i < recorder.cells.size(); i = i + 1) {
        if (recorder.cells[i] == Position{5, 3} && !recorder.values[i]) {
            seen = true;
        }
    }
    check(counter, ok && seen, "SH6.3: the cautious choice reads (5, 3) false");
}

}  // namespace

void runReadSitesTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Read sites ---\n";
    }
    checkHeuristicSite(counter);
    checkPlannerSite(counter);
    checkCautiousSite(counter);
}

}  // namespace aih
