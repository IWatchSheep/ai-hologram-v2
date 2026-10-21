// Purpose: Implements parseWorld, reading a world from ten
// lines of text with the original's checks in the original's
// order (plan Section 3.5).
// Introduced in: Stage 1a (rebuild)
// Architecture: Small helpers in an unnamed namespace do one
// job each: rejecting a world, testing border cells, applying
// one character and scanning all cells. parseWorld itself
// runs those steps in the approved order and stops at the
// first error, returning its exact message.

#include "world/world_text.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "world/world_state.h"

namespace aih {
namespace {

// An unnamed namespace keeps these helper names private to
// this source file, so no other file can depend on them.

// Builds the result for bad input: no world, one message.
// std::nullopt is the value that means "no world is here".
ParseResult rejectedWorld(const std::string& message) {
    return {std::nullopt, message};
}

// A border cell is on the outer edge of the grid.
bool isBorderCell(int x, int y) {
    return x == 0 || x == gridSize - 1 || y == 0 || y == gridSize - 1;
}

// How many agents, objects and destinations have been seen.
struct EntityCounts {
    int agents = 0;
    int objects = 0;
    int destinations = 0;
};

// Every line must be exactly gridSize characters long. All
// lengths are checked before any character is read.
bool haveCorrectLineLengths(const std::vector<std::string>& lines) {
    for (const std::string& line : lines) {
        if (line.size() != static_cast<std::size_t>(gridSize)) {
            return false;
        }
    }
    return true;
}

// Records one non-border character (or a border wall) in the
// state and the counts, or returns the unknown-character
// message. A later agent, object or destination overwrites
// an earlier position, exactly as the original does.
std::string applyCellCharacter(char cell, int x, int y, WorldState& state,
                               EntityCounts& counts) {
    switch (cell) {
        case '#':
            state.walls[static_cast<std::size_t>(y)]
                       [static_cast<std::size_t>(x)] = true;
            break;
        case '.':
            break;
        case 'A':
            counts.agents = counts.agents + 1;
            state.agent = {x, y};
            break;
        case 'O':
            counts.objects = counts.objects + 1;
            state.object = {x, y};
            break;
        case 'D':
            counts.destinations = counts.destinations + 1;
            state.destination = {x, y};
            break;
        case '*':
            counts.objects = counts.objects + 1;
            counts.destinations = counts.destinations + 1;
            state.object = {x, y};
            state.destination = {x, y};
            break;
        case '+':
            counts.agents = counts.agents + 1;
            counts.destinations = counts.destinations + 1;
            state.agent = {x, y};
            state.destination = {x, y};
            break;
        default:
            return "The world contains an unknown character.";
    }
    return "";
}

// Checks one cell: a border cell that is not a wall is the
// border error; anything else goes to applyCellCharacter.
std::string parseCell(char cell, int x, int y, WorldState& state,
                      EntityCounts& counts) {
    if (isBorderCell(x, y) && cell != '#') {
        return "Every border cell must be a wall (#).";
    }
    return applyCellCharacter(cell, x, y, state, counts);
}

// Reads cells row by row (y from 0 to 9) and left to right
// within a row (x from 0 to 9), stopping at the first error.
std::string parseCells(const std::vector<std::string>& lines, WorldState& state,
                       EntityCounts& counts) {
    for (int y = 0; y < gridSize; y = y + 1) {
        for (int x = 0; x < gridSize; x = x + 1) {
            const char cell =
                lines[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)];
            const std::string error = parseCell(cell, x, y, state, counts);
            if (!error.empty()) {
                return error;
            }
        }
    }
    return "";
}

}  // namespace

ParseResult parseWorld(const std::vector<std::string>& lines) {
    if (lines.size() != static_cast<std::size_t>(gridSize)) {
        return rejectedWorld("A world must contain exactly 10 lines.");
    }
    if (!haveCorrectLineLengths(lines)) {
        return rejectedWorld(
            "Every world line must contain exactly 10 characters.");
    }
    WorldState state{};
    EntityCounts counts;
    const std::string cellError = parseCells(lines, state, counts);
    if (!cellError.empty()) {
        return rejectedWorld(cellError);
    }
    if (counts.agents != 1) {
        return rejectedWorld(
            "A world must contain exactly one agent (A or +).");
    }
    if (counts.objects != 1) {
        return rejectedWorld(
            "A world must contain exactly one object (O or *).");
    }
    if (counts.destinations != 1) {
        return rejectedWorld(
            "A world must contain exactly one destination (D, * or +).");
    }
    if (!isValidWorld(state)) {
        return rejectedWorld("The parsed world state is not valid.");
    }
    return {std::optional<WorldState>{state}, ""};
}

}  // namespace aih
