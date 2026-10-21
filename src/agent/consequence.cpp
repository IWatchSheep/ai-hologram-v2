// Purpose: Implements checkConsequence (plan 3.7): what it
// means when the search is exhausted with the object where
// it is.
// Introduced in: Stage 3d (rebuild)
// Architecture: This file is the planned split from
// investigation.cpp (plan Section 4): the consequence check
// has its own job. It compares the stored dead status of the
// object cell, read only through readDeadStatus (IE-1 H4),
// with the free one-cell local check at that cell.

#include "agent/investigation.h"

#include <vector>

#include "agent/dead_squares.h"
#include "agent/history_checks.h"
#include "agent/observation.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

ConsequenceOutcome checkConsequence(const WorldState& belief,
                                    const DeadSquareMap& map,
                                    const std::vector<Observation>& history,
                                    FreeComputationCounts& counts) {
    const Position p = belief.object;
    if (!isInsideGrid(p)) {
        return ConsequenceOutcome::NotApplicable;
    }
    if (readDeadStatus(map, p)) {
        return ConsequenceOutcome::NotApplicable;
    }
    if (p == belief.destination) {
        return ConsequenceOutcome::NotApplicable;
    }
    // The local check re-derives "not dead" from the walls
    // around p. When it says not dead, the full state (agent
    // and object together) may still be unsolvable: that is
    // a full-state dead end, and no claim is made.
    if (localCheckSaysNotDead(belief, map, p, counts)) {
        return ConsequenceOutcome::FullStateDeadEnd;
    }
    if (!trapHistoryPasses(p, belief.destination, history)) {
        return ConsequenceOutcome::NotApplicable;
    }
    return ConsequenceOutcome::Mismatch;
}

}  // namespace aih
