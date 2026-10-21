// Purpose: Tests for the footprint disagreement table
// (FP2 to FP9, plan Section 6.1, one check per row).
// Introduced in: Stage 3a (rebuild)
// Architecture: Each row runs one step from a belief and a
// true world, then compares cells, outcomes and every
// difference record, in order, with all six values.
// Expected values come from FP_ROWS in checks.py.

#include <cstddef>
#include <string>
#include <vector>

#include "agent/footprint.h"
#include "agent_test_support.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

// Builds one expected difference from its six values.
FootprintCellDifference expectedDiff(Position cell, bool predictedWall,
                                     bool predictedAgent, bool predictedObject,
                                     bool observedWall, bool observedAgent,
                                     bool observedObject) {
    FootprintCellDifference difference;
    difference.cell = cell;
    difference.wallDiffers = predictedWall != observedWall;
    difference.agentDiffers = predictedAgent != observedAgent;
    difference.objectDiffers = predictedObject != observedObject;
    difference.predictedWall = predictedWall;
    difference.observedWall = observedWall;
    difference.predictedAgentHere = predictedAgent;
    difference.observedAgentHere = observedAgent;
    difference.predictedObjectHere = predictedObject;
    difference.observedObjectHere = observedObject;
    return difference;
}

// Returns true when two difference records agree in every field.
bool sameDifference(const FootprintCellDifference& left,
                    const FootprintCellDifference& right) {
    return left.cell == right.cell && left.wallDiffers == right.wallDiffers &&
           left.agentDiffers == right.agentDiffers &&
           left.objectDiffers == right.objectDiffers &&
           left.predictedWall == right.predictedWall &&
           left.observedWall == right.observedWall &&
           left.predictedAgentHere == right.predictedAgentHere &&
           left.observedAgentHere == right.observedAgentHere &&
           left.predictedObjectHere == right.predictedObjectHere &&
           left.observedObjectHere == right.observedObjectHere;
}

// Runs one table row and records one check for it.
void checkRow(TestCounter& counter, const std::string& description,
              const WorldState& belief, const WorldState& truth, Action action,
              const std::vector<Position>& expectedCells,
              MoveOutcome expectedPredicted, MoveOutcome expectedObserved,
              const std::vector<FootprintCellDifference>& expectedDiffs) {
    const ActionResult predicted = applyAction(belief, action);
    const ActionResult observed = applyAction(truth, action);
    const std::vector<Position> cells =
        computeFootprint(belief, action, observed.state, observed.outcome);
    const FootprintComparison comparison =
        compareFootprint(predicted.state, predicted.outcome, observed.state,
                         observed.outcome, cells);
    bool passed =
        cells == expectedCells && predicted.outcome == expectedPredicted &&
        observed.outcome == expectedObserved &&
        comparison.outcomesDiffer == (expectedPredicted != expectedObserved) &&
        comparison.cells.size() == expectedDiffs.size();
    if (passed) {
        for (std::size_t i = 0; i < expectedDiffs.size(); i = i + 1) {
            if (!sameDifference(comparison.cells[i], expectedDiffs[i])) {
                passed = false;
            }
        }
    }
    check(counter, passed, description);
}

void checkFp2Fp3(TestCounter& counter) {
    checkRow(counter, "FP2: North from D, no differences", demoWorld(),
             demoWorld(), Action::North, {Position{3, 3}, Position{3, 2}},
             MoveOutcome::Moved, MoveOutcome::Moved, {});
    const WorldState truth = withWallAt(demoWorld(), Position{4, 3}, true);
    checkRow(
        counter, "FP3: East into a true wall at (4, 3)", demoWorld(), truth,
        Action::East, {Position{3, 3}, Position{4, 3}}, MoveOutcome::Moved,
        MoveOutcome::Blocked,
        {expectedDiff(Position{3, 3}, false, false, false, false, true, false),
         expectedDiff(Position{4, 3}, false, true, false, true, false, false)});
}

void checkFp4Fp5(TestCounter& counter) {
    const WorldState belief = withWallAt(demoWorld(), Position{4, 3}, true);
    checkRow(
        counter, "FP4: East believed blocked, truly free", belief, demoWorld(),
        Action::East, {Position{3, 3}, Position{4, 3}}, MoveOutcome::Blocked,
        MoveOutcome::Moved,
        {expectedDiff(Position{3, 3}, false, true, false, false, false, false),
         expectedDiff(Position{4, 3}, true, false, false, false, true, false)});
    const WorldState start = withAgentAt(demoWorld(), Position{4, 3});
    const WorldState blockedBelief = withWallAt(start, Position{6, 3}, true);
    checkRow(
        counter, "FP5: East push believed blocked at (6, 3)", blockedBelief,
        start, Action::East, {Position{4, 3}, Position{5, 3}, Position{6, 3}},
        MoveOutcome::Blocked, MoveOutcome::Pushed,
        {expectedDiff(Position{4, 3}, false, true, false, false, false, false),
         expectedDiff(Position{5, 3}, false, false, true, false, true, false),
         expectedDiff(Position{6, 3}, true, false, false, false, false, true)});
}

void checkFp6Fp7(TestCounter& counter) {
    const WorldState start = withAgentAt(demoWorld(), Position{4, 3});
    const WorldState belief = withObjectAt(start, Position{5, 4});
    checkRow(counter, "FP6: object believed at (5, 4), truly pushed", belief,
             start, Action::East,
             {Position{4, 3}, Position{5, 3}, Position{6, 3}},
             MoveOutcome::Moved, MoveOutcome::Pushed,
             {expectedDiff(Position{6, 3}, false, false, false, false, false,
                           true)});
    const WorldState walledBelief = withWallAt(belief, Position{6, 3}, true);
    const WorldState walledTruth = withWallAt(start, Position{6, 3}, true);
    checkRow(
        counter, "FP7: push cell walled, object seen at c1", walledBelief,
        walledTruth, Action::East,
        {Position{4, 3}, Position{5, 3}, Position{6, 3}}, MoveOutcome::Moved,
        MoveOutcome::Blocked,
        {expectedDiff(Position{4, 3}, false, false, false, false, true, false),
         expectedDiff(Position{5, 3}, false, true, false, false, false, true)});
}

void checkFp8Fp9(TestCounter& counter) {
    const WorldState start = withAgentAt(demoWorld(), Position{4, 3});
    const WorldState truth = withObjectAt(start, Position{5, 4});
    checkRow(counter, "FP8: East believed push, truly a move", start, truth,
             Action::East, {Position{4, 3}, Position{5, 3}, Position{6, 3}},
             MoveOutcome::Pushed, MoveOutcome::Moved,
             {expectedDiff(Position{6, 3}, false, false, true, false, false,
                           false)});
    const WorldState movedTruth = withAgentAt(demoWorld(), Position{3, 4});
    checkRow(
        counter, "FP9: North, agent truly at (3, 4)", demoWorld(), movedTruth,
        Action::North, {Position{3, 3}, Position{3, 2}}, MoveOutcome::Moved,
        MoveOutcome::Moved,
        {expectedDiff(Position{3, 3}, false, false, false, false, true, false),
         expectedDiff(Position{3, 2}, false, true, false, false, false,
                      false)});
}

}  // namespace

void runFootprintRowsTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Footprint rows ---\n";
    }
    checkFp2Fp3(counter);
    checkFp4Fp5(counter);
    checkFp6Fp7(counter);
    checkFp8Fp9(counter);
}

}  // namespace aih
