// Purpose: Declares parsing and rendering for the plain-text
// grid format of exactly 10 lines of 10 characters.
// Introduced in: Stage 1a (rebuild)
// Architecture: Text layouts are the simple input and output
// for world states. Parsing lives in world_parse.cpp and
// rendering in world_render.cpp: two separate jobs, so the
// original world_text.cpp is split in advance (plan Section 4).

#ifndef AI_HOLOGRAM_SRC_WORLD_WORLD_TEXT_H
#define AI_HOLOGRAM_SRC_WORLD_WORLD_TEXT_H

#include <optional>
#include <string>
#include <vector>

#include "world/world_state.h"

namespace aih {

// The result of parsing: either a world, or an error message.
struct ParseResult {
    // std::optional holds a world on success, or no value at
    // all when parsing fails. The error text is empty on
    // success and explains the first problem on failure.
    std::optional<WorldState> world;
    std::string error;
};

// Reads exactly ten rows of ten characters, checking in the
// exact order of plan Section 3.5 and returning the first
// error found, with the original's exact message.
// std::vector is a resizable list; here it holds text rows.
ParseResult parseWorld(const std::vector<std::string>& lines);

// Converts a valid world back into its ten text lines, in the
// order of plan Section 3.5. It requires a valid state and
// indexes walls only with its own loop counters 0 to 9.
std::vector<std::string> renderWorld(const WorldState& state);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_WORLD_WORLD_TEXT_H
