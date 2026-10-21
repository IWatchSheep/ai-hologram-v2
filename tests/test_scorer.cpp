// Purpose: Tests for the scorer's run scores and for
// claimValueIsTrue (SC1, SC2, SC3; plan Section 6.3).
// Introduced in: Stage 4d (rebuild)
// Architecture: Expected values are copied from checks.py
// check_scorer. SC1 scores the one fault of FR1, SC2 scores
// a hand-written error with no fault events, and SC3 judges
// one claim of each kind on its own, including the object
// claim judged at its moment (D-042).

#include <vector>

#include "agent/observation.h"
#include "agent/records.h"
#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
#include "experiment/fault_event.h"
#include "experiment/fault_run.h"
#include "experiment/scorer.h"
#include "fault_test_material.h"
#include "stage4_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

// The one fault of FR1 and SC1, as checks.py DEMO1 builds it.
FaultEvent demo1Event() {
    FaultEvent event = handBuiltEvent("demo-1", 1, FaultType::F1);
    event.direction = F1Direction::BlockedToFree;
    event.hasCell = true;
    event.cell = Position{4, 3};
    return event;
}

void checkSc1(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    check(counter, isValidWorld(d2), "SC1: D2 valid");
    SingleAgent agent(makeObservation(1, d2), AgentSettings{});
    const FaultRunResult run = runWithFaults(
        d2, agent, AgentSettings{}, std::vector<FaultEvent>{demo1Event()});
    bool ok = run.eventScores.size() == 1;
    if (ok) {
        const EventScore& score = run.eventScores[0];
        ok = ok && score.activated && score.activationStep == 1;
        ok = ok && score.revealing;
        ok = ok && score.opportunityStep.has_value();
        ok = ok && score.opportunityStep.value() == 1;
        ok = ok && score.mismatchStep.has_value();
        ok = ok && score.mismatchStep.value() == 2;
        ok = ok && score.strictStep.has_value();
        ok = ok && score.strictStep.value() == 2;
        ok = ok && score.endState == EndState::Corrected;
        ok = ok && score.endStep == 2 && score.correctingId == 1;
        ok = ok && score.correctionStatus.has_value();
        ok = ok && score.correctionStatus.value() == CorrectionStatus::Verified;
    }
    check(counter, ok, "SC1: the event");
    const RunScore& score = run.score;
    ok = score.claims == 1 && score.falseAlarms == 0;
    ok = ok && score.wrongAccepted == 0 && score.mismatches == 1;
    ok = ok && score.mismatchesWithoutCause == 0;
    ok = ok && score.labelsCorrect == 1 && score.labelsScored == 1;
    ok = ok && score.justifiedSafeStops == 0;
    check(counter, ok, "SC1: the run");
}

void checkSc2(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    check(counter, isValidWorld(demo) && isValidWorld(d2), "SC2: D, D2 valid");
    SingleAgent agent(makeObservation(1, d2), AgentSettings{}, demo);
    const FaultRunResult run =
        runWithFaults(d2, agent, AgentSettings{}, std::vector<FaultEvent>{});
    const RunScore& score = run.score;
    bool ok = score.claims == 1 && score.falseAlarms == 1;
    ok = ok && score.wrongAccepted == 0 && score.mismatches == 1;
    ok = ok && score.mismatchesWithoutCause == 1;
    ok = ok && score.labelsScored == 0 && score.labelsCorrect == 0;
    check(counter, ok, "SC2: no events, a hand-written error");
}

CorrectionRecord claimOf(ElementKind kind, Position cell) {
    CorrectionRecord record;
    record.id = 1;
    record.element.kind = kind;
    record.element.cell = cell;
    return record;
}

void checkSc3Claims(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    check(counter, isValidWorld(demo) && isValidWorld(d2), "SC3: D, D2 valid");
    CorrectionRecord wall = claimOf(ElementKind::Wall, Position{4, 3});
    wall.newBlocked = true;
    bool ok = claimValueIsTrue(wall, d2, {}, {});
    ok = ok && !claimValueIsTrue(wall, demo, {}, {});
    check(counter, ok, "SC3: the wall claim");
    CorrectionRecord model = claimOf(ElementKind::ActionModel, Position{0, 0});
    model.element.action = Action::East;
    check(counter, claimValueIsTrue(model, demo, {}, {}),
          "SC3: the model claim");
    CorrectionRecord trap = claimOf(ElementKind::Trap, Position{1, 1});
    ok = claimValueIsTrue(trap, demo, {}, {});
    trap.element.cell = Position{5, 3};
    ok = ok && !claimValueIsTrue(trap, demo, {}, {});
    check(counter, ok, "SC3: the trap claims (1, 1) and (5, 3)");
}

void checkSc3Object(TestCounter& counter) {
    const WorldState demo = demoWorld();
    WorldState second = demo;
    second.agent = Position{4, 3};
    WorldState third = second;
    third.agent = Position{5, 3};
    third.object = Position{6, 3};
    check(counter, isValidWorld(second) && isValidWorld(third),
          "SC3: the object history valid");
    std::vector<Observation> history;
    history.push_back(makeObservation(1, demo));
    Observation secondObs = makeObservation(2, second);
    secondObs.lastAction = Action::East;
    secondObs.lastOutcome = MoveOutcome::Moved;
    history.push_back(secondObs);
    Observation thirdObs = makeObservation(3, third);
    thirdObs.lastAction = Action::East;
    thirdObs.lastOutcome = MoveOutcome::Pushed;
    history.push_back(thirdObs);
    MismatchRecord mismatch;
    mismatch.decisionStep = 3;
    mismatch.action = Action::East;
    mismatch.suspected = SourceLabel::S2;
    CorrectionRecord object = claimOf(ElementKind::Object, Position{0, 0});
    object.newPosition = Position{5, 3};
    bool ok = claimValueIsTrue(object, demo, history, {mismatch});
    object.newPosition = Position{6, 3};
    ok = ok && !claimValueIsTrue(object, demo, history, {mismatch});
    ok = ok && !claimValueIsTrue(object, demo, history, {});
    check(counter, ok, "SC3: the object claim judged at its moment (D-042)");
}

}  // namespace

void runScorerTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Scorer ---\n";
    }
    checkSc1(counter);
    checkSc2(counter);
    checkSc3Claims(counter);
    checkSc3Object(counter);
}

}  // namespace aih
