// Purpose: Implements describeDifferences, the exact text
// lines of plan Section 3.5.
// Introduced in: Stage 2b (rebuild)
// Architecture: Small private helpers write a position and
// a wall state once each, and the append helpers build the
// lines in comparison order without changing the input.

#include "compare/difference_text.h"

#include <string>
#include <vector>

#include "compare/state_difference.h"
#include "world/position.h"

namespace aih {
namespace {

// Writes a position as "(x, y)".
std::string positionText(const Position& position) {
    return "(" + std::to_string(position.x) + ", " +
           std::to_string(position.y) + ")";
}

// Writes a wall state as "blocked" or "free".
std::string wallText(bool blocked) {
    if (blocked) {
        return "blocked";
    }
    return "free";
}

// Adds the line for one position, but only when it differs.
void appendPositionDifference(std::vector<std::string>& lines,
                              const std::string& name,
                              const PositionDifference& difference) {
    if (difference.differs) {
        lines.push_back("  - " + name + ": predicted " +
                        positionText(difference.predicted) + ", observed " +
                        positionText(difference.observed));
    }
}

// Adds one line per differing wall, in list order.
void appendWallDifferences(std::vector<std::string>& lines,
                           const std::vector<WallDifference>& walls) {
    for (const WallDifference& wall : walls) {
        lines.push_back("  - wall at " + positionText(wall.cell) +
                        ": predicted " + wallText(wall.predictedBlocked) +
                        ", observed " + wallText(wall.observedBlocked));
    }
}

}  // namespace

std::vector<std::string> describeDifferences(
    const StateDifference& difference) {
    std::vector<std::string> lines;
    appendPositionDifference(lines, "agent", difference.agent);
    appendPositionDifference(lines, "object", difference.object);
    appendPositionDifference(lines, "destination", difference.destination);
    appendWallDifferences(lines, difference.walls);
    return lines;
}

}  // namespace aih
