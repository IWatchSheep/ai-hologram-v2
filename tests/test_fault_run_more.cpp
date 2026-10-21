// Purpose: More tests for the run with faults and for
// onObservation (FR5, FR6, FR7, RL2, RL3; plan 6.2).
// Introduced in: Stage 4c (rebuild)
// Architecture: Expected values are copied from checks.py
// check_fault_runs and check_run_watchers. onObservation
// only watches: RL2 and RL3 prove the seen observations
// and that a watched run is unchanged. FR7's extra
// watchers are the fault run's own watch-only watchers.

#include <utility>
#include <vector>

#include "agent/agent_types.h"
#include "agent/naive_agent.h"
#include "agent/observation.h"
#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
#include "expected_demo_lines.h"
#include "experiment/fault_event.h"
#include "experiment/fault_injector.h"
#include "experiment/fault_run.h"
#include "fault_test_material.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

FaultEvent demo1Event() {
    FaultEvent event = handBuiltEvent("demo-1", 1, FaultType::F1);
    event.direction = F1Direction::BlockedToFree;
    event.hasCell = true;
    event.cell = Position{4, 3};
    return event;
}

bool sameActivations(const std::vector<ActivationRecord>& left,
                     const std::vector<ActivationRecord>& right) {
    bool ok = left.size() == right.size();
    for (std::size_t i = 0; ok && i < left.size(); i = i + 1) {
        ok = ok && left[i].eventId == right[i].eventId;
        ok = ok && left[i].status == right[i].status;
        ok = ok && left[i].scheduledStep == right[i].scheduledStep;
        ok = ok && left[i].applicationStep == right[i].applicationStep;
        ok = ok && left[i].reason == right[i].reason;
    }
    return ok;
}

FaultRunResult demoRun5(const FaultTestMaterial& material) {
    const WorldState& world = material.layouts[5];
    SingleAgent agent(makeObservation(1, world), AgentSettings{});
    return runWithFaults(world, agent, AgentSettings{}, material.events[5]);
}

void checkFr5Fr6(TestCounter& counter) {
    const FaultTestMaterial material = demonstrationFaultMaterial();
    const FaultRunResult first = demoRun5(material);
    const FaultRunResult second = demoRun5(material);
    bool ok = first.run.lines == second.run.lines;
    ok = ok && first.run.outcome == second.run.outcome;
    ok = ok && first.decisionSteps == second.decisionSteps;
    ok = ok && sameActivations(first.activations, second.activations);
    check(counter, ok, "FR5: demonstration run 500002-2 (C1) twice identical");
    const WorldState demo = demoWorld();
    WorldState goal = demo;
    goal.object = goal.destination;
    check(counter, isValidWorld(goal),
          "FR6: D with the object on the destination is valid");
    SingleAgent agent(makeObservation(1, goal), AgentSettings{});
    FaultEvent event = handBuiltEvent("t", 1, FaultType::F2);
    event.order = {Action::North, Action::East, Action::South, Action::West};
    const FaultRunResult run = runWithFaults(goal, agent, AgentSettings{},
                                             std::vector<FaultEvent>{event});
    ok = run.run.outcome == "Success" && run.run.worldSteps == 0;
    ok = ok && run.decisionSteps == 0 && run.activations.size() == 1;
    if (run.activations.size() == 1) {
        ok = ok && reportedAs(run.activations[0]) ==
                       "Not activated: run ended before the scheduled step";
    }
    check(counter, ok,
          "FR6: no decision is made and the event stays scheduled");
}

void checkFr7(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    std::vector<std::pair<char, int>> calls;
    FaultRunWatchers watchers;
    watchers.afterDecision = [&calls](const SingleAgent&,
                                      const Observation& observation,
                                      const Decision&, const FaultInjector&) {
        calls.push_back(std::make_pair('d', observation.decisionStep));
    };
    watchers.afterFinalCheck = [&calls](const SingleAgent&,
                                        const Observation& observation,
                                        const FaultInjector&) {
        calls.push_back(std::make_pair('f', observation.decisionStep));
    };
    SingleAgent agent(makeObservation(1, d2), AgentSettings{});
    const FaultRunResult run =
        runWithFaults(d2, agent, AgentSettings{},
                      std::vector<FaultEvent>{demo1Event()}, watchers);
    SingleAgent plainAgent(makeObservation(1, d2), AgentSettings{});
    const FaultRunResult plain = runWithFaults(
        d2, plainAgent, AgentSettings{}, std::vector<FaultEvent>{demo1Event()});
    std::vector<std::pair<char, int>> expected;
    for (int step = 1; step <= 18; step = step + 1) {
        expected.push_back(std::make_pair('d', step));
    }
    expected.push_back(std::make_pair('f', 19));
    bool ok = calls == expected && run.run.lines == plain.run.lines;
    ok = ok && run.run.evaluations == plain.run.evaluations;
    check(
        counter, ok,
        "FR7: extra watchers at decision steps 1-18 and the final check (19)");
}

void checkRl2(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    std::vector<std::pair<int, Position>> seen;
    SingleAgent agent(makeObservation(1, d2), AgentSettings{}, demo);
    RunWatchers watchers;
    watchers.onObservation = [&seen](const SingleAgent&,
                                     const Observation& observation) {
        seen.push_back(
            std::make_pair(observation.decisionStep, observation.state.object));
    };
    const RunResult run = runSingleAgent(d2, agent, AgentSettings{}, watchers);
    bool ok = seen.size() == 19;
    for (std::size_t i = 0; ok && i < seen.size(); i = i + 1) {
        ok = ok && seen[i].first == static_cast<int>(i) + 1;
    }
    if (!seen.empty()) {
        ok = ok && seen.back().second == d2.destination;
    }
    ok = ok && linesEqual(c1Demo2Lines(), run.lines, *counter.out);
    check(counter, ok,
          "RL2: onObservation called 19 times, steps 1-19, lines exactly C1_2");
}

void checkRl3(TestCounter& counter) {
    const WorldState demo = demoWorld();
    std::vector<int> seen;
    NaiveAgent agent(makeObservation(1, demo), AgentSettings{});
    NaiveRunWatchers watchers;
    watchers.onObservation = [&seen](const NaiveAgent&,
                                     const Observation& observation) {
        seen.push_back(observation.decisionStep);
    };
    const RunResult run =
        runSingleAgent(demo, agent, AgentSettings{}, watchers);
    bool ok = seen == std::vector<int>{1, 2, 3, 4};
    ok = ok && run.outcome == "Success" && run.worldSteps == 4;
    ok = ok && run.evaluations == 16;
    check(
        counter, ok,
        "RL3: C0 onObservation at steps 1-4; Success, 4 steps, 16 evaluations");
}

}  // namespace

void runFaultRunMoreTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Fault run more ---\n";
    }
    checkFr5Fr6(counter);
    checkFr7(counter);
    checkRl2(counter);
    checkRl3(counter);
}

}  // namespace aih
