// Purpose: Implements direction names and movement offsets.
// Introduced in: Stage 1a (rebuild)
// Architecture: These helpers support the world model and, in
// Stage 1b, the movement rules. offsetFor is the one place that
// knows how each direction changes a position (rule R4).

#include "world/action.h"

#include <string>

#include "world/position.h"

namespace aih {

std::string actionName(Action action) {
    // A switch selects a result based on which enum value was
    // provided. Every case returns, so no case falls through.
    switch (action) {
        case Action::North: {
            return "North";
        }
        case Action::East: {
            return "East";
        }
        case Action::South: {
            return "South";
        }
        case Action::West: {
            return "West";
        }
    }
    return "Unknown";
}

Position offsetFor(Action action) {
    switch (action) {
        case Action::North: {
            return {0, -1};
        }
        case Action::East: {
            return {1, 0};
        }
        case Action::South: {
            return {0, 1};
        }
        case Action::West: {
            return {-1, 0};
        }
    }
    return {0, 0};
}

Position moved(Position position, Action action) {
    // The offset is const because this function only reads it;
    // it builds and returns a brand new position instead.
    const Position offset = offsetFor(action);
    return {position.x + offset.x, position.y + offset.y};
}

}  // namespace aih
