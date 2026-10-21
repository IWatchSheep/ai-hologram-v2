// Purpose: Declares the one fixed order in which directions
// are tried: North, East, South, West.
// Introduced in: Stage 3b (rebuild)
// Architecture: Every loop over directions in agent code uses
// this one list (rule R4), so the dead-square analysis, the
// local check, the planner and the tests all try directions
// in exactly the same order as the original agent.

#ifndef AI_HOLOGRAM_SRC_AGENT_DIRECTIONS_H
#define AI_HOLOGRAM_SRC_AGENT_DIRECTIONS_H

#include <array>

#include "world/action.h"

namespace aih {

// The four actions, always in the order North, East, South,
// West. constexpr means this list is fixed at compile time
// and can never be changed while the program runs.
constexpr std::array<Action, 4> kActionOrder = {Action::North, Action::East,
                                                Action::South, Action::West};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_DIRECTIONS_H
