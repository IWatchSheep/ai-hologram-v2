// Purpose: Implements the three history checks of plan 3.7.
// Introduced in: Stage 3d (rebuild)
// Architecture: The history is the list of observations C1
// was delivered, in order; nothing ever changes an entry, so
// these checks only read it.

#include "agent/history_checks.h"

#include <cstddef>
#include <vector>

#include "agent/observation.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

bool wallHistoryCheck(Position cell, bool blocked,
                      const std::vector<Observation>& history) {
    for (const Observation& entry : history) {
        if (isWall(entry.state, cell) != blocked) {
            return false;
        }
    }
    return true;
}

bool objectHistoryCheck(Position value, const Observation& before) {
    // Only the observation before the action (o_t-1) is
    // checked: the object moves from step to step, so where
    // it was earlier says nothing about where it was then.
    return before.state.object == value;
}

bool trapHistoryPasses(Position p, Position destination,
                       const std::vector<Observation>& history) {
    for (std::size_t i = 0; i < history.size(); i = i + 1) {
        if (history[i].state.object != p) {
            continue;
        }
        for (std::size_t j = i + 1; j < history.size(); j = j + 1) {
            if (history[j].state.object == destination) {
                return false;
            }
        }
    }
    return true;
}

}  // namespace aih
