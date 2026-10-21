// Purpose: Declares the three checks of a candidate or a
// trap claim against the recorded history.
// Introduced in: Stage 3d (rebuild)
// Architecture: Each check is the one copy of its rule
// (rule R4): the investigation uses the wall and object
// checks, and the consequence and trap checks share the
// trap history check (the original had two copies).

#ifndef AI_HOLOGRAM_SRC_AGENT_HISTORY_CHECKS_H
#define AI_HOLOGRAM_SRC_AGENT_HISTORY_CHECKS_H

#include <vector>

#include "agent/observation.h"
#include "world/position.h"

namespace aih {

// True when every history entry has that wall value at cell.
// A wall never moves, so the whole history must agree.
bool wallHistoryCheck(Position cell, bool blocked,
                      const std::vector<Observation>& history);

// True when the observation before the action recorded the
// object at value. The object's history check is time-aware:
// it looks only at that one observation (o_t-1), because the
// object moves, so an older entry may rightly show it
// somewhere else.
bool objectHistoryCheck(Position value, const Observation& before);

// False when some entry has the object at p and a later
// entry has it on the destination: then the object did
// escape p, so p cannot be a dead square.
bool trapHistoryPasses(Position p, Position destination,
                       const std::vector<Observation>& history);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_HISTORY_CHECKS_H
