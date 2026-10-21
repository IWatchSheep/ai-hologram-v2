// Purpose: Tests for state comparison (CM1 to CM6, plan
// Section 6.3).
// Introduced in: Stage 2b (rebuild)
// Architecture: Every check starts from the parsed OPEN
// layout and changes one thing at a time, so a failure
// points at the one part of the comparison that changed.
// Expected values come from checks.py, the reference
// mirror in tools/reference/stage2/.

#include <cstddef>

#include "compare/state_difference.h"
#include "test_layouts.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/position.h"
#include "world/world_state.h"
#include "world/world_text.h"

namespace aih {
namespace {

// Returns the parsed OPEN world, or an empty state on failure.
WorldState parsedOpen() {
    const ParseResult result = parseWorld(openLayout());
    if (result.world.has_value()) {
        return result.world.value();
    }
    return WorldState{};
}

// Returns a copy of state with the wall at (x, y) flipped.
// The checks pass only interior cells holding nothing, so
// every fixture stays a valid world, as the plan requires.
WorldState withWallFlipped(const WorldState& state, int x, int y) {
    WorldState copy = state;
    const std::size_t row = static_cast<std::size_t>(y);
    const std::size_t column = static_cast<std::size_t>(x);
    copy.walls[row][column] = !copy.walls[row][column];
    return copy;
}

void checkIdenticalAndAgent(TestCounter& counter) {
    const WorldState base = parsedOpen();
    const StateDifference same = compareStates(base, base);
    check(counter, !hasDifferences(same),
          "CM1: base vs a copy has no differences");
    WorldState moved = base;
    moved.agent = Position{4, 3};
    const StateDifference diff = compareStates(base, moved);
    const bool passed =
        diff.agent.differs && diff.agent.predicted == Position{4, 4} &&
        diff.agent.observed == Position{4, 3} && !diff.object.differs &&
        !diff.destination.differs && diff.walls.empty();
    check(counter, passed, "CM2: only the agent differs, (4, 4) to (4, 3)");
}

void checkObjectAndDestination(TestCounter& counter) {
    const WorldState base = parsedOpen();
    WorldState objectMoved = base;
    objectMoved.object = Position{7, 6};
    const StateDifference objectDiff = compareStates(base, objectMoved);
    const bool objectPassed =
        !objectDiff.agent.differs && objectDiff.object.differs &&
        objectDiff.object.predicted == Position{7, 7} &&
        objectDiff.object.observed == Position{7, 6} &&
        !objectDiff.destination.differs && objectDiff.walls.empty();
    check(counter, objectPassed,
          "CM3: only the object differs, (7, 7) to (7, 6)");
    WorldState destinationMoved = base;
    destinationMoved.destination = Position{2, 8};
    const StateDifference destinationDiff =
        compareStates(base, destinationMoved);
    const bool destinationPassed =
        !destinationDiff.agent.differs && !destinationDiff.object.differs &&
        destinationDiff.destination.differs &&
        destinationDiff.destination.predicted == Position{1, 8} &&
        destinationDiff.destination.observed == Position{2, 8} &&
        destinationDiff.walls.empty();
    check(counter, destinationPassed,
          "CM4: only the destination differs, (1, 8) to (2, 8)");
}

void checkSingleWall(TestCounter& counter) {
    const WorldState base = parsedOpen();
    const WorldState walled = withWallFlipped(base, 5, 5);
    const StateDifference forward = compareStates(walled, base);
    const bool forwardPassed =
        forward.walls.size() == 1 && forward.walls[0].cell == Position{5, 5} &&
        forward.walls[0].predictedBlocked && !forward.walls[0].observedBlocked;
    check(counter, forwardPassed,
          "CM5a: one wall, (5, 5) predicted blocked, observed free");
    const StateDifference reverse = compareStates(base, walled);
    const bool reversePassed =
        reverse.walls.size() == 1 && reverse.walls[0].cell == Position{5, 5} &&
        !reverse.walls[0].predictedBlocked && reverse.walls[0].observedBlocked;
    check(counter, reversePassed,
          "CM5b: one wall, (5, 5) predicted free, observed blocked");
}

void checkWallOrder(TestCounter& counter) {
    const WorldState base = parsedOpen();
    WorldState predicted = withWallFlipped(base, 5, 4);
    predicted = withWallFlipped(predicted, 4, 5);
    WorldState observed = base;
    observed.agent = Position{4, 3};
    const StateDifference diff = compareStates(predicted, observed);
    // Row 4 comes before row 5, so (5, 4) is reported
    // before (4, 5) even though its x is larger. Reporting
    // column by column would swap them and fail this check.
    bool passed = diff.agent.differs && !diff.object.differs &&
                  !diff.destination.differs && diff.walls.size() == 2;
    if (passed) {
        passed =
            diff.walls[0].cell == Position{5, 4} &&
            diff.walls[0].predictedBlocked && !diff.walls[0].observedBlocked &&
            diff.walls[1].cell == Position{4, 5} &&
            diff.walls[1].predictedBlocked && !diff.walls[1].observedBlocked;
    }
    check(counter, passed,
          "CM6: agent differs and walls are in row order (5, 4), (4, 5)");
}

}  // namespace

void runCompareStatesTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Compare states ---\n";
    }
    checkIdenticalAndAgent(counter);
    checkObjectAndDestination(counter);
    checkSingleWall(counter);
    checkWallOrder(counter);
}

}  // namespace aih
