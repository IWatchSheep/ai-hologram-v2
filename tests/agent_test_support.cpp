// Purpose: Defines the shared test worlds and helpers for
// the agent tests.
// Introduced in: Stage 3a (rebuild)
// Architecture: The layouts are written out in full, exactly
// as in checks.py, so a reader can compare them line by
// line. Each is parsed once per call, so no test can change
// another test's world.

#include "agent_test_support.h"

#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

#include "demo/scripted_demo.h"
#include "world/position.h"
#include "world/world_state.h"
#include "world/world_text.h"

namespace aih {
namespace {

// Parses a layout into a world, or an empty state if the
// layout failed to parse. Every layout here is valid, as
// the plan requires of every fixture.
WorldState worldFromLayout(const std::vector<std::string>& layout) {
    const ParseResult result = parseWorld(layout);
    if (result.world.has_value()) {
        return result.world.value();
    }
    return WorldState{};
}

}  // namespace

WorldState demoWorld() {
    return worldFromLayout(demoLayout());
}

WorldState lpWorld() {
    return worldFromLayout(
        {"##########", "#....D##.#", "#.O......#", "#....A#..#", "#.#......#",
         "#.....##.#", "#........#", "#........#", "#........#", "##########"});
}

WorldState l2World() {
    return worldFromLayout(
        {"##########", "#....#...#", "#.O..#.A.#", "#....#...#", "#D...#...#",
         "######...#", "#........#", "#........#", "#........#", "##########"});
}

WorldState ltWorld() {
    return worldFromLayout(
        {"##########", "#O.......#", "#........#", "#........#", "#....A...#",
         "#........#", "#........#", "#.......D#", "#........#", "##########"});
}

WorldState lgWorld() {
    return worldFromLayout(
        {"##########", "#........#", "#........#", "#..A....*#", "#........#",
         "#........#", "#...##...#", "#........#", "#........#", "##########"});
}

WorldState ltaWorld() {
    return worldFromLayout(
        {"##########", "#O...A...#", "#........#", "#........#", "#........#",
         "#........#", "#........#", "#.......D#", "#........#", "##########"});
}

WorldState boxedWorld() {
    return worldFromLayout(
        {"##########", "#.#......#", "##A#.....#", "#.#..O..D#", "#........#",
         "#........#", "#...##...#", "#........#", "#........#", "##########"});
}

WorldState smallWorld() {
    return worldFromLayout(
        {"##########", "#........#", "#........#", "#..AO.D..#", "#........#",
         "#........#", "#........#", "#........#", "#........#", "##########"});
}

WorldState withAgentAt(const WorldState& world, Position agent) {
    WorldState copy = world;
    copy.agent = agent;
    return copy;
}

WorldState withObjectAt(const WorldState& world, Position object) {
    WorldState copy = world;
    copy.object = object;
    return copy;
}

WorldState withWallAt(const WorldState& world, Position cell, bool blocked) {
    WorldState copy = world;
    const std::size_t x = static_cast<std::size_t>(cell.x);
    const std::size_t y = static_cast<std::size_t>(cell.y);
    copy.walls[y][x] = blocked;
    return copy;
}

bool linesEqual(const std::vector<std::string>& expected,
                const std::vector<std::string>& actual, std::ostream& out) {
    const std::size_t common =
        expected.size() < actual.size() ? expected.size() : actual.size();
    for (std::size_t i = 0; i < common; i = i + 1) {
        if (expected[i] != actual[i]) {
            out << "first differing line " << (i + 1) << ": expected ["
                << expected[i] << "] actual [" << actual[i] << "]\n";
            return false;
        }
    }
    if (expected.size() != actual.size()) {
        out << "line counts differ: expected " << expected.size() << " actual "
            << actual.size() << "\n";
        return false;
    }
    return true;
}

}  // namespace aih
