// Purpose: Implements the layout generator of plan
// Section 3.2, with the reference's draws in the same order.
// Introduced in: Stage 4a (rebuild)
// Architecture: One SeededRandom serves the whole generation,
// attempt after attempt, so reordering one draw would change
// every later layout. Acceptance uses the experimenter's own
// search and the Stage 3 dead-square analysis of the true
// world with no overrides; an agent never sees either.

#include "experiment/layout_generator.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <vector>

#include "agent/dead_squares.h"
#include "experiment/seeded_random.h"
#include "experiment/true_search.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

// True when an interior cell other than the destination is a
// true dead square, in the analysis with no overrides.
bool hasTrueDeadInterior(const WorldState& world) {
    FreeComputationCounts counts;
    const DeadSquareMap map = computeDeadSquares(world, {}, {}, counts);
    for (int y = 1; y < gridSize - 1; y = y + 1) {
        for (int x = 1; x < gridSize - 1; x = x + 1) {
            const Position cell{x, y};
            if (cell == world.destination) {
                continue;
            }
            const std::size_t row = static_cast<std::size_t>(y);
            const std::size_t column = static_cast<std::size_t>(x);
            if (map.dead[row][column]) {
                return true;
            }
        }
    }
    return false;
}

// Draws one attempt's walls: border cells are walls with no
// draw, and every interior cell, row by row, is a wall when
// its draw below 100 is under 12.
void drawWalls(WorldState& world, SeededRandom& random) {
    for (int y = 0; y < gridSize; y = y + 1) {
        for (int x = 0; x < gridSize; x = x + 1) {
            const bool border =
                x == 0 || x == gridSize - 1 || y == 0 || y == gridSize - 1;
            const std::size_t row = static_cast<std::size_t>(y);
            const std::size_t column = static_cast<std::size_t>(x);
            if (border) {
                world.walls[row][column] = true;
            } else {
                world.walls[row][column] = random.below(100) < 12;
            }
        }
    }
}

// The free cells of the world, row by row.
std::vector<Position> freeCells(const WorldState& world) {
    std::vector<Position> free;
    for (int y = 0; y < gridSize; y = y + 1) {
        for (int x = 0; x < gridSize; x = x + 1) {
            const std::size_t row = static_cast<std::size_t>(y);
            const std::size_t column = static_cast<std::size_t>(x);
            if (!world.walls[row][column]) {
                free.push_back(Position{x, y});
            }
        }
    }
    return free;
}

// Tries one attempt. On acceptance the result is filled in
// and true is returned; otherwise nothing is accepted yet.
bool tryAttempt(SeededRandom& random, int attempt, GeneratedLayout& result) {
    WorldState world{};
    drawWalls(world, random);
    const std::vector<Position> free = freeCells(world);
    if (free.size() < 3) {
        return false;
    }
    const std::vector<Position> picked = random.pickDistinct(free, 3);
    world.agent = picked[0];
    world.object = picked[1];
    world.destination = picked[2];
    if (!isValidWorld(world)) {
        return false;
    }
    const std::optional<int> shortest = shortestSolutionLength(world);
    if (!shortest.has_value()) {
        return false;
    }
    if (shortest.value() < 12 || shortest.value() > 40) {
        return false;
    }
    if (!hasTrueDeadInterior(world)) {
        return false;
    }
    result.found = true;
    result.world = world;
    result.attempts = attempt;
    result.shortest = shortest.value();
    return true;
}

}  // namespace

GeneratedLayout generateLayout(std::uint64_t layoutSeed) {
    SeededRandom random(layoutSeed);
    GeneratedLayout result;
    for (int attempt = 1; attempt <= 10000; attempt = attempt + 1) {
        if (tryAttempt(random, attempt, result)) {
            return result;
        }
    }
    result.found = false;
    result.attempts = 10000;
    return result;
}

}  // namespace aih
