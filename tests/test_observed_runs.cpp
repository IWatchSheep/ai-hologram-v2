// Purpose: Tests for runs watched by the computation
// observer (SH2, SH3, SH4, SH5; plan Section 6.4).
// Introduced in: Stage 4e (rebuild)
// Architecture: Expected values are copied from checks.py
// check_observed_runs. The observer only watches: a watched
// run gives exactly the unwatched run's result (IE-T2), it
// sees every charged evaluation (IE-T3), and primary
// scoring is identical with the observer off, when nothing
// is observed (IE-T4) - primary scoring must never depend
// on the observer. SH5 is FR1's opportunity step, found by
// shadow evaluation of the observed computations.

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "agent/naive_agent.h"
#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
#include "experiment/fault_event.h"
#include "experiment/fault_run.h"
#include "experiment/scorer_types.h"
#include "fault_test_material.h"
#include "recording_observer.h"
#include "stage4_suites.h"
#include "test_support.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

// The one fault of FR1 and SH5, as checks.py DEMO1 builds it.
FaultEvent demo1Event() {
    FaultEvent event = handBuiltEvent("demo-1", 1, FaultType::F1);
    event.direction = F1Direction::BlockedToFree;
    event.hasCell = true;
    event.cell = Position{4, 3};
    return event;
}

bool sameResult(const RunResult& left, const RunResult& right) {
    bool ok = left.outcome == right.outcome;
    ok = ok && left.worldSteps == right.worldSteps;
    ok = ok && left.lines == right.lines;
    ok = ok && left.evaluations == right.evaluations;
    return ok;
}

void checkWatchedC1(TestCounter& counter, const std::string& name,
                    const WorldState& truth, std::optional<WorldState> belief) {
    RecordingObserver recorder;
    ComputationObserver observer = recorder.observer();
    SingleAgent agent(makeObservation(1, truth), AgentSettings{}, belief);
    agent.state.predictor.setObserver(&observer);
    agent.state.deadCache.observer = &observer;
    const RunResult watched = runSingleAgent(truth, agent, AgentSettings{});
    SingleAgent plainAgent(makeObservation(1, truth), AgentSettings{}, belief);
    const RunResult plain = runSingleAgent(truth, plainAgent, AgentSettings{});
    check(counter, sameResult(watched, plain),
          "SH2-" + name + ": C1 watched gives the plain result (IE-T2)");
    check(counter, recorder.evaluations == agent.totalEvaluations(),
          "SH3-" + name + ": observed equals total evaluations (IE-T3)");
}

void checkWatchedC0(TestCounter& counter, const std::string& name,
                    const WorldState& truth, std::optional<WorldState> belief) {
    RecordingObserver recorder;
    ComputationObserver observer = recorder.observer();
    NaiveAgent agent(makeObservation(1, truth), AgentSettings{}, belief);
    agent.state.predictor.setObserver(&observer);
    agent.state.deadCache.observer = &observer;
    const RunResult watched = runSingleAgent(truth, agent, AgentSettings{});
    NaiveAgent plainAgent(makeObservation(1, truth), AgentSettings{}, belief);
    const RunResult plain = runSingleAgent(truth, plainAgent, AgentSettings{});
    check(counter, sameResult(watched, plain),
          "SH2-" + name + ": C0 watched gives the plain result (IE-T2)");
    check(counter, recorder.evaluations == agent.totalEvaluations(),
          "SH3-" + name + ": observed equals total evaluations (IE-T3)");
}

FaultRunResult demoFaultRun(const FaultTestMaterial& material, int index,
                            bool single, bool observe) {
    const WorldState& world = material.layouts[static_cast<std::size_t>(index)];
    const std::vector<FaultEvent>& events =
        material.events[static_cast<std::size_t>(index)];
    if (single) {
        SingleAgent agent(makeObservation(1, world), AgentSettings{});
        return runWithFaults(world, agent, AgentSettings{}, events, observe);
    }
    NaiveAgent agent(makeObservation(1, world), AgentSettings{});
    return runWithFaults(world, agent, AgentSettings{}, events, observe);
}

// The primary scoring of checks.py primary(): everything
// except the opportunity steps the observer alone can set.
bool samePrimary(const FaultRunResult& left, const FaultRunResult& right) {
    bool ok = left.run.outcome == right.run.outcome;
    ok = ok && left.run.evaluations == right.run.evaluations;
    ok = ok && left.score.claims == right.score.claims;
    ok = ok && left.score.falseAlarms == right.score.falseAlarms;
    ok = ok && left.score.wrongAccepted == right.score.wrongAccepted;
    ok = ok && left.score.justifiedSafeStops == right.score.justifiedSafeStops;
    ok = ok && left.score.mismatches == right.score.mismatches;
    ok = ok && left.score.labelsCorrect == right.score.labelsCorrect;
    ok = ok && left.score.labelsScored == right.score.labelsScored;
    ok = ok && left.score.mismatchesWithoutCause ==
                   right.score.mismatchesWithoutCause;
    ok = ok && left.eventScores.size() == right.eventScores.size();
    for (std::size_t i = 0; ok && i < left.eventScores.size(); i = i + 1) {
        const EventScore& a = left.eventScores[i];
        const EventScore& b = right.eventScores[i];
        ok = ok && a.strictStep == b.strictStep;
        ok = ok && a.mismatchStep == b.mismatchStep;
        ok = ok && a.endState == b.endState && a.endStep == b.endStep;
        ok = ok && a.correctingId == b.correctingId;
        ok = ok && a.correctionStatus == b.correctionStatus;
    }
    return ok;
}

void checkOneDemoRun(TestCounter& counter, const FaultTestMaterial& material,
                     int index) {
    const std::string name = std::to_string(500000 + index / 2) + "-" +
                             std::to_string(index % 2 + 1);
    const FaultRunResult on1 = demoFaultRun(material, index, true, true);
    const FaultRunResult on0 = demoFaultRun(material, index, false, true);
    const FaultRunResult off1 = demoFaultRun(material, index, true, false);
    const FaultRunResult off0 = demoFaultRun(material, index, false, false);
    bool seen = on1.observedEvaluations == on1.run.evaluations;
    seen = seen && on0.observedEvaluations == on0.run.evaluations;
    check(counter, seen,
          "SH3-" + name + ": observed equals evaluations (IE-T3)");
    bool same = samePrimary(on1, off1) && samePrimary(on0, off0);
    same = same && off1.observedEvaluations == 0;
    same = same && off0.observedEvaluations == 0;
    check(counter, same,
          "SH4-" + name + ": primary scoring identical, observer off (IE-T4)");
}

void checkSh5(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    check(counter, isValidWorld(d2), "SH5a: D2 valid");
    SingleAgent agent(makeObservation(1, d2), AgentSettings{});
    const FaultRunResult run = runWithFaults(
        d2, agent, AgentSettings{}, std::vector<FaultEvent>{demo1Event()});
    bool ok = run.eventScores.size() == 1;
    if (ok) {
        ok = ok && run.eventScores[0].opportunityStep.has_value();
        ok = ok && run.eventScores[0].opportunityStep.value() == 1;
    }
    check(counter, ok, "SH5b: FR1's opportunity step is 1");
}

}  // namespace

void runObservedRunsTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Observed runs ---\n";
    }
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    check(counter, isValidWorld(demo) && isValidWorld(d2), "SH2a: D, D2 valid");
    checkWatchedC1(counter, "C1-D", demo, std::nullopt);
    checkWatchedC1(counter, "C1-D2", d2, demo);
    checkWatchedC0(counter, "C0-D", demo, std::nullopt);
    checkWatchedC0(counter, "C0-D2", d2, demo);
    const FaultTestMaterial material = demonstrationFaultMaterial();
    for (int i = 0; i < 8; i = i + 1) {
        checkOneDemoRun(counter, material, i);
    }
    checkSh5(counter);
}

}  // namespace aih
