// Purpose: Declares the cache that keeps the last dead-square
// map and reruns the analysis only when its inputs change.
// Introduced in: Stage 3b (rebuild)
// Architecture: The analysis is a free but bounded computation
// (protocol 10.7), and it is rerun only when the believed
// walls, the destination, or either override list has changed
// (plan 3.3). The map is recomputed into the same member, so
// references to it stay valid across recomputations.

#ifndef AI_HOLOGRAM_SRC_AGENT_DEAD_SQUARE_CACHE_H
#define AI_HOLOGRAM_SRC_AGENT_DEAD_SQUARE_CACHE_H

#include <array>
#include <vector>

#include "agent/dead_squares.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// Remembers the last map and the exact inputs it was computed
// from. The agent's and object's positions are not inputs of
// the analysis, so moving them alone never reruns it.
struct DeadSquareCache {
    // Returns the current map, recomputing it into the same
    // map member first if the inputs changed since the last
    // run (or there was none). Every call sets the map's
    // observer to this cache's observer.
    const DeadSquareMap& get(const WorldState& belief,
                             const std::vector<Position>& notDeadOverrides,
                             const std::vector<Position>& deadCorrections,
                             FreeComputationCounts& counts);

    DeadSquareMap map{};
    const ComputationObserver* observer = nullptr;

private:
    bool hasCachedMap_ = false;
    std::array<std::array<bool, gridSize>, gridSize> cachedWalls_{};
    Position cachedDestination_{0, 0};
    std::vector<Position> cachedNotDeadOverrides_;
    std::vector<Position> cachedDeadCorrections_;
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_DEAD_SQUARE_CACHE_H
