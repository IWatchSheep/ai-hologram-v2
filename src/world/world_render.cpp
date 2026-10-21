// Purpose: Implements renderWorld, writing a valid world
// state as ten lines of text.
// Introduced in: Stage 1a (rebuild)
// Architecture: Rendering is the reverse of parsing and lives
// in its own file (plan Section 4). For each cell the checks
// run in the order of plan Section 3.5: wall, agent on the
// destination, object on the destination, agent, object,
// destination, floor. Walls are indexed only by the loop
// counters of this function, which always run from 0 to 9.

#include "world/world_text.h"

#include <cstddef>
#include <string>
#include <vector>

#include "world/position.h"
#include "world/world_state.h"

namespace aih {

std::vector<std::string> renderWorld(const WorldState& state) {
    std::vector<std::string> lines(
        static_cast<std::size_t>(gridSize),
        std::string(static_cast<std::size_t>(gridSize), '.'));
    for (int y = 0; y < gridSize; y = y + 1) {
        for (int x = 0; x < gridSize; x = x + 1) {
            const Position cell{x, y};
            char character = '.';
            if (state.walls[static_cast<std::size_t>(y)]
                           [static_cast<std::size_t>(x)]) {
                character = '#';
            } else if (state.agent == cell && state.destination == cell) {
                character = '+';
            } else if (state.object == cell && state.destination == cell) {
                character = '*';
            } else if (state.agent == cell) {
                character = 'A';
            } else if (state.object == cell) {
                character = 'O';
            } else if (state.destination == cell) {
                character = 'D';
            } else {
                character = '.';
            }
            lines[static_cast<std::size_t>(y)][static_cast<std::size_t>(x)] =
                character;
        }
    }
    return lines;
}

}  // namespace aih
