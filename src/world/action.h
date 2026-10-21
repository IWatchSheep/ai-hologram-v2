// Purpose: Declares the four possible movement directions and
// their helpers.
// Introduced in: Stage 1a (rebuild)
// Architecture: World actions use these directions to describe
// one-cell moves. There is no "wait" action: taking no action
// means not calling the world at all.

#ifndef AI_HOLOGRAM_SRC_WORLD_ACTION_H
#define AI_HOLOGRAM_SRC_WORLD_ACTION_H

#include <string>

#include "world/position.h"

namespace aih {

// An enum class keeps these direction names grouped together
// and prevents them being mixed up with plain integers.
enum class Action { North, East, South, West };

// Returns the readable name of a direction, such as "North".
std::string actionName(Action action);

// The one shared index of an action (D-076, rule R4): North
// 0, East 1, South 2, West 3. The agent model and the fault
// model both use this, so they can never disagree.
int actionIndex(Action action);

// Returns the one-cell change for a direction: North is
// (0, -1), East is (1, 0), South is (0, 1) and West is (-1, 0).
Position offsetFor(Action action);

// Returns a new position one cell away in the given direction.
// The position passed in is not changed.
Position moved(Position position, Action action);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_WORLD_ACTION_H
