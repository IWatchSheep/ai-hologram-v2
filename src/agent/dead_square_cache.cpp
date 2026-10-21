// Purpose: Implements the dead-square cache: it reruns the
// analysis only when the believed walls, the destination, or
// either override list has changed.
// Introduced in: Stage 3b (rebuild)
// Architecture: The map is recomputed into the same map
// member, so a reference taken earlier stays valid and sees
// the new values (plan 3.3). Every call sets the map's
// observer to the cache's observer, attached or not.

#include "agent/dead_square_cache.h"

#include <vector>

#include "agent/dead_squares.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

const DeadSquareMap& DeadSquareCache::get(
    const WorldState& belief, const std::vector<Position>& notDeadOverrides,
    const std::vector<Position>& deadCorrections,
    FreeComputationCounts& counts) {
    // Only the walls, the destination and the two override
    // lists are inputs of the analysis, so only they can make
    // the cached map out of date. The override lists are
    // compared as whole vectors, in order.
    bool changed = !hasCachedMap_;
    if (belief.walls != cachedWalls_) {
        changed = true;
    }
    if (belief.destination != cachedDestination_) {
        changed = true;
    }
    if (notDeadOverrides != cachedNotDeadOverrides_) {
        changed = true;
    }
    if (deadCorrections != cachedDeadCorrections_) {
        changed = true;
    }
    if (changed) {
        map = computeDeadSquares(belief, notDeadOverrides, deadCorrections,
                                 counts);
        cachedWalls_ = belief.walls;
        cachedDestination_ = belief.destination;
        cachedNotDeadOverrides_ = notDeadOverrides;
        cachedDeadCorrections_ = deadCorrections;
        hasCachedMap_ = true;
    }
    map.observer = observer;
    return map;
}

}  // namespace aih
