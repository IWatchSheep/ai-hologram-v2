// Purpose: Declares the dead-square analysis, its stored
// map, the free-computation counters, the one way dead status
// is read, and the one-cell local check.
// Introduced in: Stage 3b (rebuild)
// Architecture: This is one of the free computations of
// protocol 10.7. It looks at object cells only, never builds
// agent-object states and never calls any model, so it is
// counted here but never charged to a BudgetCounter. A dead
// square is not a full-state dead end: a dead square says the
// object can never reach the destination from that cell even
// with relaxed pushes, while a full state (agent and object
// together) can still be unsolvable on a cell that is not
// dead, for example when the agent cannot reach the side it
// would need to push from (protocol 11.2).

#ifndef AI_HOLOGRAM_SRC_AGENT_DEAD_SQUARES_H
#define AI_HOLOGRAM_SRC_AGENT_DEAD_SQUARES_H

#include <array>
#include <vector>

#include "agent/computation_observer.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// The result of the analysis for every cell of the grid.
// dead[y][x] says whether the object can never reach the
// destination from cell (x, y) by relaxed pushes, and
// pushDistance[y][x] is the number of relaxed pushes needed,
// or -1 when the destination cannot be reached that way.
// Walls and the destination are never dead.
struct DeadSquareMap {
    std::array<std::array<bool, gridSize>, gridSize> dead{};
    std::array<std::array<int, gridSize>, gridSize> pushDistance{};
    const ComputationObserver* observer = nullptr;
};

// Counters for the free computations (protocol 10.7). These
// are counted and reported, but they are not model
// evaluations: none of these computations creates a successor
// state, so none of them uses the BudgetCounter.
struct FreeComputationCounts {
    int analysisRuns = 0;
    int cellOperations = 0;
    int localChecks = 0;
    int heuristicCalls = 0;
};

// Runs the backward breadth-first search over object cells
// only (plan 3.3), starting from the destination of belief
// and trying directions in kActionOrder. It adds 1 to
// analysisRuns and 1 to cellOperations for each (cell,
// direction) examined. Then it applies the overrides in
// order: first the "not dead" overrides, then the "dead"
// corrections, so a dead correction wins on a listed cell.
// Overrides change only the dead flag of the listed cells.
DeadSquareMap computeDeadSquares(const WorldState& belief,
                                 const std::vector<Position>& notDeadOverrides,
                                 const std::vector<Position>& deadCorrections,
                                 FreeComputationCounts& counts);

// Reads one cell's stored dead status (IE-1 H4). Every agent
// read of a dead status goes through here, except inside
// computeDeadSquares itself. When an observer is attached,
// it first reports (cell, value); the report changes nothing.
bool readDeadStatus(const DeadSquareMap& map, Position cell);

// The free one-cell check of protocol 11.4 B: true when cell
// is the destination, or some direction has the cells behind
// and ahead of it believed free and the ahead cell not dead.
// It reads walls and neighbours only and never reads the
// stored status of cell itself. It adds 1 to localChecks.
bool localCheckSaysNotDead(const WorldState& belief, const DeadSquareMap& map,
                           Position cell, FreeComputationCounts& counts);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_DEAD_SQUARES_H
