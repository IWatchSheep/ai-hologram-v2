// Purpose: Declares the shared words for positions and wall
// states, used wherever a position or a wall is written as text.
// Introduced in: Stage 3a (rebuild)
// Architecture: This is the one place these words are written
// (rule R4). The difference text and the agent log both use
// them, so every reader sees exactly the same wording.

#ifndef AI_HOLOGRAM_SRC_CORE_TEXT_WORDS_H
#define AI_HOLOGRAM_SRC_CORE_TEXT_WORDS_H

#include <string>

#include "world/position.h"

namespace aih {

// Writes a position as "(x, y)".
std::string positionText(const Position& position);

// Writes a wall state as "blocked" or "free".
std::string blockedText(bool blocked);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_CORE_TEXT_WORDS_H
