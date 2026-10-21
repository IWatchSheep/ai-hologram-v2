// Purpose: Implements the heuristic formula of plan 3.3 and
// the one Manhattan distance helper.
// Introduced in: Stage 3b (rebuild)
// Architecture: One table lookup in the DeadSquareMap plus
// two Manhattan distances. This is free but bounded
// (protocol 10.7): it is called only on states that already
// exist, it does no search and it loops over no actions, so
// it can never create a successor state. The object's dead
// status is read through readDeadStatus, and the value is
// reported to an attached observer (IE-1 H4); both reports
// only watch and change nothing.

#include "agent/heuristic.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>

#include "agent/dead_squares.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

int manhattan(Position first, Position second) {
    return std::abs(first.x - second.x) + std::abs(first.y - second.y);
}

int heuristic(const WorldState& state, const DeadSquareMap& map,
              FreeComputationCounts& counts) {
    counts.heuristicCalls = counts.heuristicCalls + 1;
    int value = kInfiniteHeuristic;
    if (!readDeadStatus(map, state.object)) {
        const std::size_t x = static_cast<std::size_t>(state.object.x);
        const std::size_t y = static_cast<std::size_t>(state.object.y);
        // P is the relaxed push count when the analysis reached
        // this cell. An overridden "not dead" cell has no
        // distance (-1), so the straight-line distance from the
        // object to the destination is used instead.
        const int pushDistance = map.pushDistance[y][x];
        int pushes = manhattan(state.object, state.destination);
        if (pushDistance >= 0) {
            pushes = pushDistance;
        }
        // The agent must first walk next to the object: standing
        // next to it (distance 1) costs no extra steps.
        const int walking =
            std::max(0, manhattan(state.agent, state.object) - 1);
        value = pushes + walking;
    }
    if (map.observer != nullptr) {
        if (map.observer->onHeuristic) {
            map.observer->onHeuristic(state, value);
        }
    }
    return value;
}

}  // namespace aih
