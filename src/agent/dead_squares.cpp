// Purpose: Implements the dead-square analysis, its
// overrides, the watched dead-status read, and the one-cell
// local check.
// Introduced in: Stage 3b (rebuild)
// Architecture: The analysis works backwards from the
// destination over object cells only. It is free but bounded
// (protocol 10.7): at most 100 cells times 4 directions are
// examined per run, it never builds agent-object states and
// never calls any model, so no successor state is created.
//
// A dead square is not the same as a full-state dead end.
// A dead square says the object can never reach the
// destination from that cell, even if the agent could stand
// anywhere: relaxed pushes ignore whether the agent can
// really walk to the pushing side. A full state (agent and
// object together) can still be unsolvable on a cell that is
// not dead, for example when the agent can never reach the
// pushing side (protocol 11.2).

#include "agent/dead_squares.h"

#include <cstddef>
#include <vector>

#include "agent/computation_observer.h"
#include "agent/directions.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

// A cell counts as free when it is not a believed wall.
// isWall already treats cells outside the grid as walls, so
// outside cells are never free.
bool isBelievedFree(const WorldState& belief, Position cell) {
    return !isWall(belief, cell);
}

// Sets the dead flag of each listed cell to deadValue, keeping
// the rule that cells outside the grid, walls and the
// destination are never dead. Nothing else about a cell
// changes: its pushDistance is left exactly as computed.
void applyOverrides(DeadSquareMap& map, const WorldState& belief,
                    const std::vector<Position>& cells, bool deadValue) {
    for (const Position cell : cells) {
        if (!isInsideGrid(cell)) {
            continue;
        }
        if (isWall(belief, cell)) {
            continue;
        }
        if (cell == belief.destination) {
            continue;
        }
        const std::size_t x = static_cast<std::size_t>(cell.x);
        const std::size_t y = static_cast<std::size_t>(cell.y);
        map.dead[y][x] = deadValue;
    }
}

// Marks every believed-free cell that the search did not
// reach as dead, except the destination itself.
void markUnreachedCellsDead(DeadSquareMap& map, const WorldState& belief) {
    for (int y = 0; y < gridSize; y = y + 1) {
        for (int x = 0; x < gridSize; x = x + 1) {
            const Position cell{x, y};
            if (isWall(belief, cell)) {
                continue;
            }
            if (cell == belief.destination) {
                continue;
            }
            const std::size_t row = static_cast<std::size_t>(y);
            const std::size_t column = static_cast<std::size_t>(x);
            map.dead[row][column] = map.pushDistance[row][column] < 0;
        }
    }
}

// Examines one (reached cell, direction) pair: q is the cell
// that could push in direction into reached, and s is the
// cell the agent would have to stand on for that push. When
// both are believed free and q has no distance yet, q gets
// one more than reached and joins the queue.
void examineDirection(const WorldState& belief, DeadSquareMap& map,
                      std::vector<Position>& queue, Position reached,
                      Action direction, FreeComputationCounts& counts) {
    counts.cellOperations = counts.cellOperations + 1;
    const Position offset = offsetFor(direction);
    const Position q{reached.x - offset.x, reached.y - offset.y};
    const Position s{reached.x - 2 * offset.x, reached.y - 2 * offset.y};
    if (!isInsideGrid(q)) {
        return;
    }
    if (!isBelievedFree(belief, q)) {
        return;
    }
    if (!isBelievedFree(belief, s)) {
        return;
    }
    const std::size_t qx = static_cast<std::size_t>(q.x);
    const std::size_t qy = static_cast<std::size_t>(q.y);
    if (map.pushDistance[qy][qx] < 0) {
        const std::size_t rx = static_cast<std::size_t>(reached.x);
        const std::size_t ry = static_cast<std::size_t>(reached.y);
        map.pushDistance[qy][qx] = map.pushDistance[ry][rx] + 1;
        queue.push_back(q);
    }
}

// The backward breadth-first search itself. The queue is a
// vector read from the front with an index, so cells are
// visited in the order they were discovered and distances
// grow one relaxed push at a time.
void runBackwardSearch(const WorldState& belief, DeadSquareMap& map,
                       FreeComputationCounts& counts) {
    std::vector<Position> queue;
    if (isInsideGrid(belief.destination)) {
        const std::size_t x = static_cast<std::size_t>(belief.destination.x);
        const std::size_t y = static_cast<std::size_t>(belief.destination.y);
        map.pushDistance[y][x] = 0;
        queue.push_back(belief.destination);
    }
    for (std::size_t front = 0; front < queue.size(); front = front + 1) {
        for (const Action direction : kActionOrder) {
            examineDirection(belief, map, queue, queue[front], direction,
                             counts);
        }
    }
}

}  // namespace

DeadSquareMap computeDeadSquares(const WorldState& belief,
                                 const std::vector<Position>& notDeadOverrides,
                                 const std::vector<Position>& deadCorrections,
                                 FreeComputationCounts& counts) {
    DeadSquareMap map;
    for (auto& row : map.pushDistance) {
        row.fill(-1);
    }
    counts.analysisRuns = counts.analysisRuns + 1;
    runBackwardSearch(belief, map, counts);
    markUnreachedCellsDead(map, belief);
    // Overrides, in this order: "not dead" first, then "dead",
    // so a dead correction wins when a cell is in both lists.
    // In Stage 3 the "not dead" list is set only by tests.
    applyOverrides(map, belief, notDeadOverrides, false);
    applyOverrides(map, belief, deadCorrections, true);
    return map;
}

bool readDeadStatus(const DeadSquareMap& map, Position cell) {
    const std::size_t x = static_cast<std::size_t>(cell.x);
    const std::size_t y = static_cast<std::size_t>(cell.y);
    const bool value = map.dead[y][x];
    if (map.observer != nullptr) {
        if (map.observer->onDeadRead) {
            map.observer->onDeadRead(cell, value);
        }
    }
    return value;
}

bool localCheckSaysNotDead(const WorldState& belief, const DeadSquareMap& map,
                           Position cell, FreeComputationCounts& counts) {
    counts.localChecks = counts.localChecks + 1;
    if (cell == belief.destination) {
        return true;
    }
    // Re-check the analysis rule at this one cell only. The
    // stored status of cell itself is deliberately never read:
    // this check tests whether that status agrees with the
    // walls and the stored statuses of its neighbours.
    for (const Action direction : kActionOrder) {
        const Position offset = offsetFor(direction);
        const Position behind{cell.x - offset.x, cell.y - offset.y};
        const Position ahead{cell.x + offset.x, cell.y + offset.y};
        if (!isBelievedFree(belief, behind)) {
            continue;
        }
        if (!isBelievedFree(belief, ahead)) {
            continue;
        }
        if (!readDeadStatus(map, ahead)) {
            return true;
        }
    }
    return false;
}

}  // namespace aih
