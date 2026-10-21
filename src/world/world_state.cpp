// Purpose: Implements comparisons and safe questions about a
// world state.
// Introduced in: Stage 1a (rebuild)
// Architecture: These functions provide the shared checks for
// the parser and renderer. World positions are int values;
// they become std::size_t array indexes only after a bounds
// check has shown they are inside the grid.

#include "world/world_state.h"

#include <cstddef>

#include "world/position.h"

namespace aih {

bool operator==(const WorldState& left, const WorldState& right) {
    return left.walls == right.walls && left.agent == right.agent &&
           left.object == right.object && left.destination == right.destination;
}

bool operator!=(const WorldState& left, const WorldState& right) {
    return !(left == right);
}

bool isInsideGrid(const Position& position) {
    return position.x >= 0 && position.x < gridSize && position.y >= 0 &&
           position.y < gridSize;
}

bool isWall(const WorldState& state, const Position& position) {
    if (!isInsideGrid(position)) {
        return true;
    }
    // The bounds check above makes this row-first access safe.
    return state.walls[static_cast<std::size_t>(position.y)]
                      [static_cast<std::size_t>(position.x)];
}

bool isValidWorld(const WorldState& state) {
    if (!isInsideGrid(state.agent)) {
        return false;
    }
    if (!isInsideGrid(state.object)) {
        return false;
    }
    if (!isInsideGrid(state.destination)) {
        return false;
    }
    for (int index = 0; index < gridSize; index = index + 1) {
        if (!state.walls[0][static_cast<std::size_t>(index)]) {
            return false;
        }
        if (!state.walls[static_cast<std::size_t>(gridSize - 1)]
                        [static_cast<std::size_t>(index)]) {
            return false;
        }
        if (!state.walls[static_cast<std::size_t>(index)][0]) {
            return false;
        }
        if (!state.walls[static_cast<std::size_t>(index)]
                        [static_cast<std::size_t>(gridSize - 1)]) {
            return false;
        }
    }
    if (isWall(state, state.agent)) {
        return false;
    }
    if (isWall(state, state.object)) {
        return false;
    }
    if (isWall(state, state.destination)) {
        return false;
    }
    return state.agent != state.object;
}

bool isGoalReached(const WorldState& state) {
    return state.object == state.destination;
}

}  // namespace aih
