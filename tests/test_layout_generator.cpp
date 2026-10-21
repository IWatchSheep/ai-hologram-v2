// Purpose: Tests for the experimenter's search and the
// layout generator (LY1 to LY4, plan Section 6.1).
// Introduced in: Stage 4a (rebuild)
// Architecture: Fixtures are the demo world of
// agent_test_support (rule R4) and layouts generated from
// the demonstration seeds. Expected values are those of
// plan 3.2 and checks.py; every description starts with its
// check id. No agent is run on any layout here.

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "agent/dead_squares.h"
#include "agent_test_support.h"
#include "experiment/layout_generator.h"
#include "experiment/true_search.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/position.h"
#include "world/world_state.h"
#include "world/world_text.h"

namespace aih {
namespace {

// The expected layout facts of checks.py LAYOUTS: seed,
// attempts, shortest, agent, object, destination, true dead
// interior cells and interior walls.
struct LayoutFact {
    std::uint64_t seed = 0;
    int attempts = 0;
    int shortest = 0;
    Position agent{0, 0};
    Position object{0, 0};
    Position destination{0, 0};
    int dead = 0;
    int walls = 0;
};

// Counts interior cells other than the destination that are
// dead in the true analysis with no overrides.
int deadInteriorCount(const WorldState& world) {
    FreeComputationCounts counts;
    const DeadSquareMap map = computeDeadSquares(world, {}, {}, counts);
    int total = 0;
    for (int y = 1; y < gridSize - 1; y = y + 1) {
        for (int x = 1; x < gridSize - 1; x = x + 1) {
            const Position cell{x, y};
            const std::size_t row = static_cast<std::size_t>(y);
            const std::size_t column = static_cast<std::size_t>(x);
            if (map.dead[row][column] && cell != world.destination) {
                total = total + 1;
            }
        }
    }
    return total;
}

// Counts the interior wall cells.
int interiorWalls(const WorldState& world) {
    int total = 0;
    for (int y = 1; y < gridSize - 1; y = y + 1) {
        for (int x = 1; x < gridSize - 1; x = x + 1) {
            const std::size_t row = static_cast<std::size_t>(y);
            const std::size_t column = static_cast<std::size_t>(x);
            if (world.walls[row][column]) {
                total = total + 1;
            }
        }
    }
    return total;
}

// True when every border cell is a wall.
bool bordersAreWalls(const WorldState& world) {
    bool bordered = true;
    for (int i = 0; i < gridSize; i = i + 1) {
        const std::size_t index = static_cast<std::size_t>(i);
        if (!world.walls[0][index]) {
            bordered = false;
        }
        if (!world.walls[gridSize - 1][index]) {
            bordered = false;
        }
        if (!world.walls[index][0]) {
            bordered = false;
        }
        if (!world.walls[index][gridSize - 1]) {
            bordered = false;
        }
    }
    return bordered;
}

void checkSearch(TestCounter& counter) {
    const WorldState demo = demoWorld();
    check(counter, isValidWorld(demo), "LY1: D valid");
    const bool shortestFour = shortestSolutionLength(demo).value_or(-1) == 4;
    check(counter, shortestFour, "LY1: D's shortest solution 4");
    const WorldState corner = withObjectAt(demo, Position{1, 1});
    check(counter, isValidWorld(corner), "LY1: D object at (1, 1) valid");
    const bool noSolution = !shortestSolutionLength(corner).has_value();
    check(counter, noSolution, "LY1: D object at (1, 1) has no solution");
    const WorldState solved = withObjectAt(demo, demo.destination);
    check(counter, isValidWorld(solved), "LY1: object on destination valid");
    const bool zeroSteps = shortestSolutionLength(solved).value_or(-1) == 0;
    check(counter, zeroSteps, "LY1: object on destination gives 0");
    const WorldState bad = withAgentAt(demo, Position{10, 3});
    const bool badOk =
        !isValidWorld(bad) && !shortestSolutionLength(bad).has_value();
    check(counter, badOk, "LY1: agent outside grid invalid and no solution");
}

// True when one generated layout matches its expected facts.
bool layoutMatches(const LayoutFact& fact) {
    const GeneratedLayout layout = generateLayout(fact.seed);
    bool matches = layout.found && layout.attempts == fact.attempts;
    matches = matches && layout.shortest == fact.shortest;
    matches = matches && layout.world.agent == fact.agent;
    matches = matches && layout.world.object == fact.object;
    matches = matches && layout.world.destination == fact.destination;
    matches = matches && deadInteriorCount(layout.world) == fact.dead;
    matches = matches && interiorWalls(layout.world) == fact.walls;
    return matches;
}

void checkLayouts(TestCounter& counter) {
    const std::vector<LayoutFact> facts{
        {500000, 3, 12, {7, 2}, {6, 5}, {2, 6}, 25, 7},
        {500001, 6, 16, {8, 1}, {4, 5}, {8, 4}, 23, 9},
        {500002, 1, 29, {4, 1}, {4, 7}, {8, 5}, 27, 8},
        {500003, 3, 14, {1, 1}, {4, 5}, {7, 4}, 26, 7},
        {900000, 2, 17, {7, 1}, {7, 6}, {2, 8}, 19, 7},
        {900001, 4, 19, {4, 3}, {3, 7}, {5, 6}, 26, 11}};
    for (const LayoutFact& fact : facts) {
        const std::string label = std::to_string(fact.seed);
        check(counter, layoutMatches(fact), "LY2: layout " + label + " exact");
    }
}

void checkDrawings(TestCounter& counter) {
    const std::vector<std::string> drawing500000{
        "##########", "#...#....#", "#.....#A.#", "##.......#", "#.#......#",
        "#.....O..#", "##D...#..#", "#........#", "#......#.#", "##########"};
    const GeneratedLayout first = generateLayout(500000);
    check(counter, renderWorld(first.world) == drawing500000,
          "LY3: drawing of layout 500000");
    const std::vector<std::string> drawing900000{
        "##########", "#....#.A##", "##...#...#", "#........#", "#.....#..#",
        "#.#......#", "#......O.#", "#......#.#", "#.D......#", "##########"};
    const GeneratedLayout pilot = generateLayout(900000);
    check(counter, renderWorld(pilot.world) == drawing900000,
          "LY3: drawing of layout 900000");
    const std::vector<int> attempts{3, 1, 3, 8, 5, 2};
    const std::vector<int> shortest{20, 16, 12, 12, 12, 16};
    bool matches = true;
    for (std::size_t i = 0; i < attempts.size(); i = i + 1) {
        const GeneratedLayout layout = generateLayout(500004 + i);
        if (layout.attempts != attempts[i] || layout.shortest != shortest[i]) {
            matches = false;
        }
    }
    check(counter, matches, "LY3: layouts 500004-500009 attempts and shortest");
}

void checkDemoLayouts(TestCounter& counter) {
    for (int seed = 500000; seed < 500010; seed = seed + 1) {
        const GeneratedLayout layout =
            generateLayout(static_cast<std::uint64_t>(seed));
        bool passed = layout.found && isValidWorld(layout.world);
        passed = passed && bordersAreWalls(layout.world);
        passed = passed && layout.shortest >= 12 && layout.shortest <= 40;
        const int searched = shortestSolutionLength(layout.world).value_or(-1);
        passed = passed && searched == layout.shortest;
        passed = passed && deadInteriorCount(layout.world) >= 1;
        const std::string label = std::to_string(seed);
        check(counter, passed, "LY4: layout " + label + " accepted properties");
    }
}

}  // namespace

void runLayoutGeneratorTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Layout generator ---\n";
    }
    checkSearch(counter);
    checkLayouts(counter);
    checkDrawings(counter);
    checkDemoLayouts(counter);
}

}  // namespace aih
