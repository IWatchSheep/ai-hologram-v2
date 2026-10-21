// Purpose: Tests for the footprint (FP1, FP10, FP11a, FP11b,
// FP12a, FP12b, FP13, plan Section 6.1).
// Introduced in: Stage 3a (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support. Expected values come from checks.py.

#include <array>
#include <vector>

#include "agent/footprint.h"
#include "agent/observation.h"
#include "agent_test_support.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

// The 30 actions of FP1_ACTIONS in checks.py.
std::array<Action, 30> fp1Actions() {
    return {Action::East,  Action::East,  Action::East,  Action::East,
            Action::East,  Action::North, Action::West,  Action::West,
            Action::West,  Action::South, Action::South, Action::East,
            Action::East,  Action::North, Action::North, Action::North,
            Action::North, Action::West,  Action::West,  Action::South,
            Action::South, Action::South, Action::South, Action::South,
            Action::South, Action::East,  Action::East,  Action::East,
            Action::West,  Action::North};
}

// FP1: after each of the 30 listed actions, a footprint
// update keeps the belief equal to the true state.
void checkFp1(TestCounter& counter) {
    const std::array<Action, 30> actions = fp1Actions();
    WorldState belief = demoWorld();
    WorldState truth = demoWorld();
    bool everyStep = true;
    int moved = 0;
    int pushed = 0;
    int blocked = 0;
    for (const Action action : actions) {
        const ActionResult after = applyAction(truth, action);
        const std::vector<Position> cells =
            computeFootprint(belief, action, after.state, after.outcome);
        applyFootprintUpdate(belief, cells, after.state);
        if (belief != after.state) {
            everyStep = false;
        }
        if (after.outcome == MoveOutcome::Moved) {
            moved = moved + 1;
        }
        if (after.outcome == MoveOutcome::Pushed) {
            pushed = pushed + 1;
        }
        if (after.outcome == MoveOutcome::Blocked) {
            blocked = blocked + 1;
        }
        truth = after.state;
    }
    const bool passed = everyStep && moved == 23 && pushed == 3 && blocked == 4;
    check(counter, passed,
          "FP1: 30 actions with footprint updates keep belief true; "
          "23 Moved, 3 Pushed, 4 Blocked");
}

void checkFp10(TestCounter& counter) {
    WorldState belief = withWallAt(demoWorld(), Position{2, 7}, true);
    const ActionResult observed = applyAction(demoWorld(), Action::East);
    const std::vector<Position> cells = computeFootprint(
        belief, Action::East, observed.state, observed.outcome);
    const ActionResult predicted = applyAction(belief, Action::East);
    const FootprintComparison comparison =
        compareFootprint(predicted.state, predicted.outcome, observed.state,
                         observed.outcome, cells);
    applyFootprintUpdate(belief, cells, observed.state);
    bool wallsObserved = true;
    for (const Position cell : cells) {
        if (isWall(belief, cell) != isWall(observed.state, cell)) {
            wallsObserved = false;
        }
    }
    const std::vector<Position> expected = {Position{3, 3}, Position{4, 3}};
    const bool passed = cells == expected && isFootprintMatch(comparison) &&
                        belief.agent == observed.state.agent &&
                        belief.object == observed.state.object &&
                        wallsObserved && isWall(belief, Position{2, 7});
    check(counter, passed,
          "FP10: cells (3, 3), (4, 3), no differences; update keeps (2, 7)");
}

void checkFp11(TestCounter& counter) {
    const WorldState first = withAgentAt(demoWorld(), Position{4, 3});
    const WorldState second = withObjectAt(first, Position{5, 4});
    const MoveOutcome firstOutcome = applyAction(first, Action::East).outcome;
    const MoveOutcome secondOutcome = applyAction(second, Action::East).outcome;
    const std::vector<Position> cells = computeFootprintForTwoPredictions(
        first, firstOutcome, second, secondOutcome, Action::East);
    const std::vector<Position> swapped = computeFootprintForTwoPredictions(
        second, secondOutcome, first, firstOutcome, Action::East);
    const std::vector<Position> expected = {Position{4, 3}, Position{5, 3},
                                            Position{6, 3}};
    const bool passedA = firstOutcome == MoveOutcome::Pushed &&
                         secondOutcome == MoveOutcome::Moved &&
                         cells == expected && swapped == expected;
    check(counter, passedA,
          "FP11a: two-prediction footprint has cells (4, 3), (5, 3), (6, 3)");
    const std::vector<Position> twoCells = {Position{4, 3}, Position{5, 3}};
    const std::vector<Position> movedCells = computeFootprintForTwoPredictions(
        second, secondOutcome, second, secondOutcome, Action::East);
    check(counter, movedCells == twoCells,
          "FP11b: two Moved predictions give cells (4, 3), (5, 3) only");
}

void checkFp12(TestCounter& counter) {
    const std::vector<Position> outside = {Position{-1, 3}, Position{10, 3},
                                           Position{3, -1}, Position{3, 10}};
    const FootprintComparison comparison =
        compareFootprint(demoWorld(), MoveOutcome::Moved, demoWorld(),
                         MoveOutcome::Moved, outside);
    WorldState copy = demoWorld();
    applyFootprintUpdate(copy, outside, demoWorld());
    bool allWalls = true;
    for (const Position cell : outside) {
        if (!isWall(demoWorld(), cell)) {
            allWalls = false;
        }
    }
    const bool passedA =
        isFootprintMatch(comparison) && allWalls && copy == demoWorld();
    check(counter, passedA,
          "FP12a: cells outside the grid are walls; update changes nothing");
    const WorldState west = withAgentAt(demoWorld(), Position{1, 3});
    const ActionResult after = applyAction(west, Action::West);
    const std::vector<Position> cells =
        computeFootprint(west, Action::West, after.state, after.outcome);
    const std::vector<Position> expected = {Position{1, 3}, Position{0, 3}};
    const bool passedB =
        cells == expected && after.outcome == MoveOutcome::Blocked;
    check(counter, passedB,
          "FP12b: agent (1, 3) West gives cells (1, 3), (0, 3), Blocked");
}

void checkFp13(TestCounter& counter) {
    const WorldState start = withAgentAt(demoWorld(), Position{4, 3});
    const ActionResult pushed = applyAction(start, Action::East);
    Observation after;
    after.decisionStep = 2;
    after.state = pushed.state;
    after.lastAction = Action::East;
    after.lastOutcome = pushed.outcome;
    const std::vector<Position> cells =
        footprintAfter(start, Action::East, after);
    const std::vector<Position> expected = {Position{4, 3}, Position{5, 3},
                                            Position{6, 3}};
    Observation noOutcome;
    noOutcome.decisionStep = 2;
    noOutcome.state = demoWorld();
    const std::vector<Position> northCells =
        footprintAfter(demoWorld(), Action::North, noOutcome);
    const std::vector<Position> expectedNorth = {Position{3, 3},
                                                 Position{3, 2}};
    const bool passed = cells == expected &&
                        observedOutcome(noOutcome) == MoveOutcome::Blocked &&
                        northCells == expectedNorth;
    check(counter, passed,
          "FP13: footprintAfter uses the observation outcome; empty is "
          "Blocked");
}

}  // namespace

void runFootprintTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Footprint ---\n";
    }
    checkFp1(counter);
    checkFp10(counter);
    checkFp11(counter);
    checkFp12(counter);
    checkFp13(counter);
}

}  // namespace aih
