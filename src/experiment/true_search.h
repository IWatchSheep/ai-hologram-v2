// Purpose: Declares the experimenter's search that finds the
// shortest solution of a true world.
// Introduced in: Stage 4a (rebuild)
// Architecture: This is experimenter code in src/experiment/.
// It searches the true world with the Stage 1 rules, which an
// agent must never see: an agent has to work out its own plan
// from its belief, within its budget. The search is never
// visible to an agent and is never charged to any budget, so
// it measures the layout without helping the agent solve it.

#ifndef AI_HOLOGRAM_SRC_EXPERIMENT_TRUE_SEARCH_H
#define AI_HOLOGRAM_SRC_EXPERIMENT_TRUE_SEARCH_H

#include <optional>

#include "world/world_state.h"

namespace aih {

// Breadth-first search over true (agent, object) states,
// trying actions in kActionOrder and skipping Blocked and
// InvalidState results. Returns the number of world steps of
// a shortest solution, 0 when the object is already on the
// destination, or no value when no solution exists.
std::optional<int> shortestSolutionLength(const WorldState& world);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_EXPERIMENT_TRUE_SEARCH_H
