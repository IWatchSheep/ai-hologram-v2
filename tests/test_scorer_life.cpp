// Purpose: Tests for the scorer's event life, driven by
// hand (SC4), and FR1's lines with the scorer (SC6).
// Introduced in: Stage 4d (rebuild)
// Architecture: Expected values are copied from checks.py
// check_scorer_life, step by step: SC4 calls the scorer's
// phase entry points itself, in the run's order, and reads
// the event's score after each step, so a claim judged at
// the wrong end of its phase is seen where it happens.

#include <vector>

#include "agent/agent_types.h"
#include "agent/observation.h"
#include "agent/records.h"
#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
#include "experiment/fault_event.h"
#include "experiment/fault_injector.h"
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

// One wall correction record at (4, 3), as checks.py
// wall_record builds it: Consistent at the given step.
CorrectionRecord wallRecord(int id, bool newBlocked, bool oldBlocked,
                            int step) {
    CorrectionRecord record;
    record.id = id;
    record.element.kind = ElementKind::Wall;
    record.element.cell = Position{4, 3};
    record.newBlocked = newBlocked;
    record.oldBlocked = oldBlocked;
    record.status = CorrectionStatus::Consistent;
    record.stepConsistent = step;
    return record;
}

// The one fault of FR1 and SC6, as checks.py DEMO1 builds it.
FaultEvent demo1Event() {
    FaultEvent event = handBuiltEvent("demo-1", 1, FaultType::F1);
    event.direction = F1Direction::BlockedToFree;
    event.hasCell = true;
    event.cell = Position{4, 3};
    return event;
}

// The hand-driven event of SC4, as checks.py event()
// builds it: step 2, F1 blocked->free at (4, 3).
FaultEvent lifeEvent() {
    FaultEvent event = handBuiltEvent("E", 2, FaultType::F1);
    event.direction = F1Direction::BlockedToFree;
    event.hasCell = true;
    event.cell = Position{4, 3};
    return event;
}

struct LifeRun {
    SingleAgent agent;
    std::vector<Observation> history;
    FaultEvent event;
    Scorer scorer;
    ScoredView view;
    std::vector<ActivationRecord> records;
    Decision action;

    explicit LifeRun(const WorldState& truth)
        : agent(makeObservation(1, truth), AgentSettings{}),
          history{makeObservation(1, truth)},
          event(lifeEvent()),
          scorer(std::vector<FaultEvent>{event}, truth, &history),
          view(viewOf(agent.state)) {
        ActivationRecord record;
        record.eventId = event.id;
        record.type = event.type;
        record.scheduledStep = event.step;
        records.push_back(record);
        action.isAction = true;
        action.action = Action::North;
    }
};

void checkSc4Start(TestCounter& counter, LifeRun& run) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    check(counter, isValidWorld(d2), "SC4: D2 valid");
    run.scorer.endPhaseA(run.view, 1);
    run.scorer.afterActivation(run.view, run.records, 1);
    run.scorer.afterDecide(run.view, run.action);
    run.agent.state.corrections.push_back(wallRecord(1, true, true, 2));
    run.scorer.endPhaseA(run.view, 2);
    const EventScore& score = run.scorer.eventScores()[0];
    bool ok = run.scorer.runScore().falseAlarms == 1;
    ok = ok && run.scorer.runScore().claims == 1 && !score.activated;
    check(counter, ok, "SC4: a claim before activation is a false alarm");
}

void checkSc4Activate(TestCounter& counter, LifeRun& run) {
    run.agent.state.belief.walls[3][4] = false;
    run.records[0].status = ActivationStatus::Applied;
    run.records[0].applicationStep = 2;
    run.scorer.afterActivation(run.view, run.records, 2);
    const EventScore& score = run.scorer.eventScores()[0];
    bool ok = score.activated && score.activationStep == 2 && score.active;
    check(counter, ok, "SC4: activated at step 2 and active");
    run.scorer.afterDecide(run.view, run.action);
    run.agent.state.corrections.push_back(wallRecord(2, true, false, 3));
    run.agent.state.belief.walls[3][4] = true;
    run.scorer.endPhaseA(run.view, 3);
}

void checkSc4Corrected(TestCounter& counter, LifeRun& run) {
    const EventScore& score = run.scorer.eventScores()[0];
    bool ok = score.strictStep.has_value() && score.strictStep.value() == 3;
    ok = ok && score.endState == EndState::Corrected && score.endStep == 3;
    ok = ok && score.correctingId == 2;
    ok = ok && run.scorer.runScore().falseAlarms == 1;
    check(counter, ok, "SC4: corrected at step 3");
    run.scorer.afterActivation(run.view, run.records, 3);
    run.scorer.afterDecide(run.view, run.action);
    run.agent.state.corrections[1].status = CorrectionStatus::Rejected;
    run.agent.state.belief.walls[3][4] = false;
    run.scorer.endPhaseA(run.view, 4);
}

void checkSc4Revived(TestCounter& counter, LifeRun& run) {
    const EventScore& score = run.scorer.eventScores()[0];
    bool ok = score.endState == EndState::None && score.active;
    ok = ok && score.correctingId == 0;
    check(counter, ok, "SC4: rejected correction makes the event active again");
    run.scorer.afterActivation(run.view, run.records, 4);
    Decision stop;
    stop.isAction = false;
    stop.action = Action::North;
    stop.stopReason = "test";
    run.scorer.afterDecide(run.view, stop);
    check(counter, run.scorer.runScore().justifiedSafeStops == 1,
          "SC4: a safe stop while active is justified");
    run.scorer.endFinalCheck(run.view);
    run.scorer.finish(run.view);
}

void checkSc4End(TestCounter& counter, const LifeRun& run) {
    const EventScore& score = run.scorer.eventScores()[0];
    bool ok = score.endState == EndState::None;
    ok = ok && score.strictStep.has_value() && score.strictStep.value() == 3;
    ok = ok && run.scorer.runScore().claims == 2;
    ok = ok && run.scorer.runScore().falseAlarms == 1;
    ok = ok && run.scorer.runScore().wrongAccepted == 0;
    ok = ok && !score.correctionStatus.has_value();
    check(counter, ok, "SC4: the totals at the end");
}

void checkSc4(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    LifeRun run(d2);
    checkSc4Start(counter, run);
    checkSc4Activate(counter, run);
    checkSc4Corrected(counter, run);
    checkSc4Revived(counter, run);
    checkSc4End(counter, run);
}

void checkSc6(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    check(counter, isValidWorld(demo) && isValidWorld(d2), "SC6: D, D2 valid");
    SingleAgent agent(makeObservation(1, d2), AgentSettings{});
    const FaultRunResult run = runWithFaults(
        d2, agent, AgentSettings{}, std::vector<FaultEvent>{demo1Event()});
    SingleAgent plainAgent(makeObservation(1, d2), AgentSettings{}, demo);
    const RunResult plain = runSingleAgent(d2, plainAgent, AgentSettings{});
    bool ok = run.run.lines == plain.lines && run.run.outcome == plain.outcome;
    ok = ok && run.run.evaluations == plain.evaluations;
    ok = ok && !agent.state.activationHook;
    check(counter, ok, "SC6: FR1 lines unchanged with the scorer, hook empty");
}

}  // namespace

void runScorerLifeTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Scorer life ---\n";
    }
    checkSc4(counter);
    checkSc6(counter);
}

}  // namespace aih
