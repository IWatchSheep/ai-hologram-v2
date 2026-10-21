// Purpose: Tests for the IE-1 H4 observer reports from dead
// squares and the heuristic (OB1, OB2, OB3, plan Section 6.2).
// Introduced in: Stage 3b (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld. A recording
// observer only watches: its events are compared exactly
// with the expected reports of checks.py (OB2_EVENTS), and
// each description starts with its check id.

#include <array>
#include <string>
#include <vector>

#include "agent/computation_observer.h"
#include "agent/dead_square_cache.h"
#include "agent/dead_squares.h"
#include "agent/heuristic.h"
#include "agent_test_support.h"
#include "recording_observer.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

void checkOb1(TestCounter& counter) {
    const WorldState demo = demoWorld();
    FreeComputationCounts counts;
    DeadSquareMap map = computeDeadSquares(demo, {}, {}, counts);
    const bool silentFirst = readDeadStatus(map, Position{1, 1});
    const bool silentSecond = readDeadStatus(map, Position{5, 3});
    RecordingObserver recorder;
    ComputationObserver observer = recorder.observer();
    map.observer = &observer;
    const bool watchedFirst = readDeadStatus(map, Position{1, 1});
    const bool watchedSecond = readDeadStatus(map, Position{5, 3});
    const std::vector<std::string> expected = {"dead (1, 1) true",
                                               "dead (5, 3) false"};
    const bool values =
        silentFirst && !silentSecond && watchedFirst && !watchedSecond;
    check(counter, isValidWorld(demo) && values && recorder.events == expected,
          "OB1: readDeadStatus reports (1, 1) true, then (5, 3) false");
}

void checkOb2(TestCounter& counter) {
    const WorldState demo = demoWorld();
    FreeComputationCounts counts;
    DeadSquareMap map = computeDeadSquares(demo, {}, {}, counts);
    RecordingObserver recorder;
    ComputationObserver observer = recorder.observer();
    map.observer = &observer;
    FreeComputationCounts freeCounts;
    heuristic(demo, map, freeCounts);
    const WorldState stuck = withObjectAt(demo, Position{1, 1});
    heuristic(stuck, map, freeCounts);
    localCheckSaysNotDead(demo, map, Position{2, 2}, freeCounts);
    const std::vector<std::string> expected = {
        "dead (5, 3) false", "heuristic (3, 3) (5, 3) 4",
        "dead (1, 1) true",  "heuristic (3, 3) (1, 1) 1000000000",
        "dead (2, 1) true",  "dead (3, 2) false"};
    const bool valid = isValidWorld(demo) && isValidWorld(stuck);
    check(counter, valid && recorder.events == expected,
          "OB2: heuristic and local check report exactly OB2_EVENTS");
}

void checkOb3(TestCounter& counter) {
    const WorldState demo = demoWorld();
    FreeComputationCounts counts;
    DeadSquareCache cache;
    const DeadSquareMap& plain = cache.get(demo, {}, {}, counts);
    const std::array<std::array<bool, gridSize>, gridSize> plainDead =
        plain.dead;
    const ComputationObserver* plainObserver = plain.observer;
    RecordingObserver recorder;
    ComputationObserver observer = recorder.observer();
    cache.observer = &observer;
    const DeadSquareMap& watched = cache.get(demo, {}, {}, counts);
    const bool passed = isValidWorld(demo) && plainObserver == nullptr &&
                        watched.observer == &observer &&
                        watched.dead == plainDead;
    check(counter, passed,
          "OB3: cache get sets map.observer on every call; dead unchanged");
}

}  // namespace

void runDeadSquareObserversTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Dead-square observers ---\n";
    }
    checkOb1(counter);
    checkOb2(counter);
    checkOb3(counter);
}

}  // namespace aih
