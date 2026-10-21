// Purpose: Tests for the dead-square analysis (DS1, DS2a,
// DS2b, DS3a, DS4, DS5a, DS5b, plan Section 6.2).
// Introduced in: Stage 3b (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld. Expected
// values come from checks.py, and each description starts
// with its check id.

#include <array>
#include <vector>

#include "agent/dead_squares.h"
#include "agent_test_support.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

// The full grid of DS2_GRID in checks.py: -2 is a wall, -3 is
// a dead cell, and any other number is the pushDistance.
std::array<std::array<int, gridSize>, gridSize> ds2Grid() {
    return {{{-2, -2, -2, -2, -2, -2, -2, -2, -2, -2},
             {-2, -3, -3, -3, -3, -3, -3, -3, -3, -2},
             {-2, -3, 7, 6, 5, 4, 3, 2, 1, -2},
             {-2, -3, 6, 5, 4, 3, 2, 1, 0, -2},
             {-2, -3, 7, 6, 5, 4, 3, 2, 1, -2},
             {-2, -3, 8, 7, 6, 5, 4, 3, 2, -2},
             {-2, -3, 9, 8, -2, -2, 5, 4, 3, -2},
             {-2, -3, 10, 9, 8, 7, 6, 5, 4, -2},
             {-2, -3, -3, -3, -3, -3, -3, -3, -3, -2},
             {-2, -2, -2, -2, -2, -2, -2, -2, -2, -2}}};
}

// True when one cell of the map matches its DS2_GRID entry.
bool cellMatches(const WorldState& demo, const DeadSquareMap& map, int x, int y,
                 int want) {
    const std::size_t row = static_cast<std::size_t>(y);
    const std::size_t column = static_cast<std::size_t>(x);
    if (want == -2) {
        return demo.walls[row][column] && !map.dead[row][column];
    }
    if (want == -3) {
        return map.dead[row][column] && map.pushDistance[row][column] == -1;
    }
    return !map.dead[row][column] && map.pushDistance[row][column] == want;
}

void checkDs1AndDs2(TestCounter& counter) {
    const WorldState demo = demoWorld();
    FreeComputationCounts counts;
    const DeadSquareMap map = computeDeadSquares(demo, {}, {}, counts);
    bool noWallDead = true;
    for (int y = 0; y < gridSize; y = y + 1) {
        for (int x = 0; x < gridSize; x = x + 1) {
            if (demo.walls[y][x] && map.dead[y][x]) {
                noWallDead = false;
            }
        }
    }
    const bool ds1 = isValidWorld(demo) && map.dead[1][1] && !map.dead[3][8] &&
                     map.pushDistance[3][8] == 0 && noWallDead;
    check(counter, ds1,
          "DS1: (1, 1) dead; destination (8, 3) not dead, distance 0; "
          "no wall dead");
    const std::array<std::array<int, gridSize>, gridSize> grid = ds2Grid();
    bool gridOk = true;
    int deadCount = 0;
    for (int y = 0; y < gridSize; y = y + 1) {
        for (int x = 0; x < gridSize; x = x + 1) {
            if (map.dead[y][x]) {
                deadCount = deadCount + 1;
            }
            if (!cellMatches(demo, map, x, y, grid[y][x])) {
                gridOk = false;
            }
        }
    }
    check(counter, gridOk && deadCount == 22,
          "DS2a: every cell matches DS2_GRID; exactly 22 dead cells");
    const bool ds2b = counts.analysisRuns == 1 && counts.cellOperations == 160;
    check(counter, ds2b, "DS2b: one analysis run and 160 cell operations");
}

void checkDs3(TestCounter& counter) {
    const WorldState l2 = l2World();
    FreeComputationCounts counts;
    const DeadSquareMap map = computeDeadSquares(l2, {}, {}, counts);
    const bool passed =
        isValidWorld(l2) && !map.dead[2][2] && map.pushDistance[2][2] == 3;
    check(counter, passed,
          "DS3a: L2 object cell (2, 2) not dead, pushDistance 3");
}

void checkDs4AndDs5(TestCounter& counter) {
    const WorldState demo = demoWorld();
    FreeComputationCounts counts;
    const DeadSquareMap plain = computeDeadSquares(demo, {}, {}, counts);
    FreeComputationCounts overCounts;
    const DeadSquareMap over =
        computeDeadSquares(demo, {Position{1, 1}}, {}, overCounts);
    const bool at11 = localCheckSaysNotDead(demo, over, Position{1, 1}, counts);
    const bool at53 = localCheckSaysNotDead(demo, over, Position{5, 3}, counts);
    const bool at83 = localCheckSaysNotDead(demo, over, Position{8, 3}, counts);
    const bool ds4 =
        isValidWorld(demo) && !at11 && at53 && at83 && counts.localChecks == 3;
    check(counter, ds4,
          "DS4: local check false at (1, 1), true at (5, 3) and (8, 3); "
          "localChecks 3");
    int differCount = 0;
    Position differCell{-1, -1};
    for (int y = 0; y < gridSize; y = y + 1) {
        for (int x = 0; x < gridSize; x = x + 1) {
            if (plain.dead[y][x] != over.dead[y][x]) {
                differCount = differCount + 1;
                differCell = Position{x, y};
            }
        }
    }
    const bool ds5a = differCount == 1 && differCell == Position{1, 1} &&
                      plain.pushDistance == over.pushDistance;
    check(counter, ds5a,
          "DS5a: not-dead override changes exactly (1, 1), no distance");
    FreeComputationCounts bothCounts;
    const DeadSquareMap both = computeDeadSquares(demo, {Position{1, 1}},
                                                  {Position{1, 1}}, bothCounts);
    check(counter, both.dead[1][1],
          "DS5b: dead correction on the same cell wins (applied last)");
}

}  // namespace

void runDeadSquaresTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Dead squares ---\n";
    }
    checkDs1AndDs2(counter);
    checkDs3(counter);
    checkDs4AndDs5(counter);
}

}  // namespace aih
