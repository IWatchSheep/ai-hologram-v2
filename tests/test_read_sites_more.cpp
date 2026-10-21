// Purpose: More tests for the dead-status read sites
// reported to the observer: the consequence check, the
// local check and the trap V2 test (SH6; plan Section 6.4).
// Introduced in: Stage 4e (rebuild)
// Architecture: Expected values are copied from checks.py
// check_read_sites. Every agent read of a dead status goes
// through readDeadStatus (IE-1 H4), so an attached observer
// sees exactly these reads, and every reported value equals
// the map's stored value. The local check deliberately
// never reads the stored status of the cell it checks: it
// reads the cell ahead instead, which is why DF4e-1 (reading
// map.dead directly there) silences exactly its reports.

#include <cstddef>
#include <optional>
#include <vector>

#include "agent/dead_squares.h"
#include "agent/investigation.h"
#include "agent/step_predictor.h"
#include "agent/verification.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
#include "model/world_model.h"
#include "recording_observer.h"
#include "stage4_suites.h"
#include "test_support.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

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

// True when the recorder saw cell read with value false.
bool sawNotDead(const RecordingObserver& recorder, Position cell) {
    for (std::size_t i = 0; i < recorder.cells.size(); i = i + 1) {
        if (recorder.cells[i] == cell && !recorder.values[i]) {
            return true;
        }
    }
    return false;
}

void checkConsequenceSite(TestCounter& counter) {
    const WorldState demo = demoWorld();
    FreeComputationCounts counts;
    DeadSquareMap map = computeDeadSquares(demo, {}, {}, counts);
    RecordingObserver recorder;
    ComputationObserver observer = recorder.observer();
    map.observer = &observer;
    const ConsequenceOutcome outcome =
        checkConsequence(demo, map, {makeObservation(1, demo)}, counts);
    bool ok = outcome == ConsequenceOutcome::FullStateDeadEnd;
    const std::vector<Position> expected = {Position{5, 3}, Position{5, 2}};
    ok = ok && recorder.cells == expected;
    ok = ok && recorder.values == std::vector<bool>({false, false});
    ok = ok && valuesMatch(recorder, map);
    check(counter, ok,
          "SH6.4: consequence check reads (5, 3) and (5, 2), both false");
}

void checkLocalSite(TestCounter& counter) {
    const WorldState demo = demoWorld();
    FreeComputationCounts counts;
    DeadSquareMap map = computeDeadSquares(demo, {}, {}, counts);
    RecordingObserver recorder;
    ComputationObserver observer = recorder.observer();
    map.observer = &observer;
    const bool notDead =
        localCheckSaysNotDead(demo, map, Position{5, 3}, counts);
    bool ok = notDead;
    ok = ok && recorder.cells == std::vector<Position>{Position{5, 2}};
    ok = ok && recorder.values == std::vector<bool>{false};
    ok = ok && valuesMatch(recorder, map);
    check(counter, ok, "SH6.5: the local check reads (5, 2) false only");
}

void checkTrapSite(TestCounter& counter) {
    const WorldState demo = demoWorld();
    FreeComputationCounts counts;
    DeadSquareMap map = computeDeadSquares(demo, {}, {}, counts);
    RecordingObserver recorder;
    ComputationObserver observer = recorder.observer();
    map.observer = &observer;
    StepPredictor predictor(WorldModel(), 200);
    predictor.startStep();
    int anomalies = 0;
    const TrapV2Result result = trapV2Test(demo, map, Position{5, 3}, predictor,
                                           applyAction, 1, anomalies);
    bool ok = result == TrapV2Result::Rejected;
    ok = ok && sawNotDead(recorder, Position{6, 3});
    ok = ok && valuesMatch(recorder, map);
    check(counter, ok, "SH6.6: the trap V2 test reads (6, 3) false");
}

}  // namespace

void runReadSitesMoreTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Read sites more ---\n";
    }
    checkConsequenceSite(counter);
    checkLocalSite(counter);
    checkTrapSite(counter);
}

}  // namespace aih
