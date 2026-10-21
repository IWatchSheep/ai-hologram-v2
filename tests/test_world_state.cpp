// Purpose: Tests for isValidWorld, isWall outside the grid
// and actionName (V1 to V5, V6a to V6d and N1, plan 6.4).
// Introduced in: Stage 1a (rebuild)
// Architecture: Each check starts from the parsed DEMO
// layout and changes exactly one thing, so a failure points
// at the one rule that stopped working.

#include "test_layouts.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"
#include "world/world_text.h"

namespace aih {
namespace {

// Returns the parsed DEMO world, or an empty state on failure.
WorldState parsedDemo() {
    const ParseResult result = parseWorld(demoLayout());
    if (result.world.has_value()) {
        return result.world.value();
    }
    return WorldState{};
}

void checkValidity(TestCounter& counter) {
    const WorldState demo = parsedDemo();
    check(counter, isValidWorld(demo), "V1: parsed DEMO is valid");
    WorldState onWall = demo;
    onWall.agent = {4, 6};
    check(counter, !isValidWorld(onWall),
          "V2: DEMO with agent on a wall at (4,6) is invalid");
    WorldState sameCell = demo;
    sameCell.agent = demo.object;
    check(counter, !isValidWorld(sameCell),
          "V3: DEMO with agent on the object cell is invalid");
    WorldState openBorder = demo;
    openBorder.walls[0][5] = false;
    check(counter, !isValidWorld(openBorder),
          "V4: DEMO with walls[0][5] cleared is invalid");
    WorldState agentOutside = demo;
    agentOutside.agent = {10, 3};
    WorldState objectOutside = demo;
    objectOutside.object = {3, -1};
    check(counter, !isValidWorld(agentOutside) && !isValidWorld(objectOutside),
          "V5: DEMO with agent (10,3) and with object (3,-1) are invalid");
}

void checkIsWallOutside(TestCounter& counter) {
    const WorldState demo = parsedDemo();
    check(counter, isWall(demo, Position{-1, 0}),
          "V6a: isWall is true for (-1,0) outside the grid");
    check(counter, isWall(demo, Position{10, 0}),
          "V6b: isWall is true for (10,0) outside the grid");
    check(counter, isWall(demo, Position{0, -1}),
          "V6c: isWall is true for (0,-1) outside the grid");
    check(counter, isWall(demo, Position{0, 10}),
          "V6d: isWall is true for (0,10) outside the grid");
}

}  // namespace

void runWorldStateTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- World state ---\n";
    }
    checkValidity(counter);
    checkIsWallOutside(counter);
    check(counter,
          actionName(Action::North) == "North" &&
              actionName(Action::East) == "East" &&
              actionName(Action::South) == "South" &&
              actionName(Action::West) == "West",
          "N1: actionName gives North, East, South, West");
}

}  // namespace aih
