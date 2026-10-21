// Purpose: Implements the shared words for positions and
// wall states.
// Introduced in: Stage 3a (rebuild)
// Architecture: These small builders were moved here from
// difference_text.cpp unchanged (plan 3.12, rule R4), so
// there is still only one copy of each.

#include "core/text_words.h"

#include <string>

#include "world/position.h"

namespace aih {

std::string positionText(const Position& position) {
    return "(" + std::to_string(position.x) + ", " +
           std::to_string(position.y) + ")";
}

std::string blockedText(bool blocked) {
    if (blocked) {
        return "blocked";
    }
    return "free";
}

}  // namespace aih
