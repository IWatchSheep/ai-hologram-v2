// Purpose: Declares the plain-text description of a state
// difference, one line per difference.
// Introduced in: Stage 2b (rebuild)
// Architecture: This is the one place positions and wall
// states are written as text (rule R4), so every reader of
// a difference sees exactly the same wording.

#ifndef AI_HOLOGRAM_SRC_COMPARE_DIFFERENCE_TEXT_H
#define AI_HOLOGRAM_SRC_COMPARE_DIFFERENCE_TEXT_H

#include <string>
#include <vector>

#include "compare/state_difference.h"

namespace aih {

// Writes one line per difference in the order of the
// comparison: agent, object, destination, then walls.
// Each line is two spaces, "- ", then the difference, with
// positions as "(x, y)" and walls as "blocked" or "free".
std::vector<std::string> describeDifferences(const StateDifference& difference);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_COMPARE_DIFFERENCE_TEXT_H
