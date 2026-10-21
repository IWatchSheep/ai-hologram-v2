// Purpose: Declares the grid world state and the functions
// that inspect it safely.
// Introduced in: Stage 1a (rebuild)
// Architecture: This is the shared representation used by the
// parser, the renderer and (in Stage 1b) the movement rules.
// isValidWorld is the one definition of a valid state (R4).

#ifndef AI_HOLOGRAM_SRC_WORLD_WORLD_STATE_H
#define AI_HOLOGRAM_SRC_WORLD_WORLD_STATE_H

#include <array>

#include "world/position.h"

namespace aih {

// The grid is always 10 columns by 10 rows.
constexpr int gridSize = 10;

// One complete description of a world at one moment.
struct WorldState {
    // std::array is a fixed-size list. Each inner array is one
    // row, so walls[y][x] means row y, column x. This row-first
    // order matches how a grid is read from top to bottom.
    // The empty braces value-initialize the walls to false and
    // the positions to (0, 0).
    std::array<std::array<bool, gridSize>, gridSize> walls{};
    Position agent{};
    Position object{};
    Position destination{};
};

// Equality lets code compare all positions and wall cells at
// once, which the tests use to show a state did not change.
bool operator==(const WorldState& left, const WorldState& right);
bool operator!=(const WorldState& left, const WorldState& right);

// Returns whether both coordinates are valid grid indexes.
// Parameters passed by const reference are read but never
// changed, and are not copied.
bool isInsideGrid(const Position& position);

// Returns true for any position outside the grid, and reads
// the walls array only after checking the position is inside.
// This is the one place outside-the-grid handling lives (R4).
bool isWall(const WorldState& state, const Position& position);

// Checks the single approved definition of a valid world
// state (plan Section 3.6).
bool isValidWorld(const WorldState& state);

// Returns whether the object is on the destination.
bool isGoalReached(const WorldState& state);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_WORLD_WORLD_STATE_H
