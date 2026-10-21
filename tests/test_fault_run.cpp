// Purpose: Tests for the run with faults (FR1, FR4,
// IE-T1; plan Section 6.2): the FR1 fault, the empty event
// list, and the fault-free demonstrations unchanged.
// Introduced in: Stage 4c (rebuild)
// Architecture: Expected values are copied from checks.py
// check_fault_runs. The run with faults reuses the Stage 3
// loop through watchers, so with no events it must give
// exactly runSingleAgent's result; FR1's one fault turns
// its run into Stage 3 demonstration 2 exactly.

#include <optional>
#include <vector>

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
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

// The one fault of FR1, as checks.py DEMO1 builds it.
FaultEvent demo1Event() {
    FaultEvent event = handBuiltEvent("demo-1", 1, FaultType::F1);
    event.direction = F1Direction::BlockedToFree;
    event.hasCell = true;
    event.cell = Position{4, 3};
    return event;
}

bool sameRun(const FaultRunResult& fault, const RunResult& plain) {
    bool ok = fault.run.outcome == plain.outcome;
    ok = ok && fault.run.worldSteps == plain.worldSteps;
    ok = ok && fault.run.lines == plain.lines;
    ok = ok && fault.run.evaluations == plain.evaluations;
    return ok;
}

void checkFr1(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    check(counter, isValidWorld(demo) && isValidWorld(d2), "FR1: D, D2 valid");
    SingleAgent plainAgent(makeObservation(1, d2), AgentSettings{}, demo);
    const RunResult plain = runSingleAgent(d2, plainAgent, AgentSettings{});
    SingleAgent agent(makeObservation(1, d2), AgentSettings{});
    const FaultRunResult run = runWithFaults(
        d2, agent, AgentSettings{}, std::vector<FaultEvent>{demo1Event()});
    bool ok = sameRun(run, plain) && run.run.evaluations == 414;
    ok = ok && run.activations.size() == 1;
    if (run.activations.size() == 1) {
        ok = ok && run.activations[0].status == ActivationStatus::Applied;
        ok = ok && run.activations[0].applicationStep == 1;
        ok = ok && reportedAs(run.activations[0]) ==
                       "Activated at its scheduled step";
    }
    check(counter, ok,
          "FR1: demo-1 gives demonstration 2 exactly, 414 evaluations");
}

bool emptyC1(const WorldState& truth, std::optional<WorldState> belief) {
    SingleAgent faultAgent(makeObservation(1, truth), AgentSettings{}, belief);
    const FaultRunResult fault =
        runWithFaults(truth, faultAgent, AgentSettings{}, {});
    SingleAgent plainAgent(makeObservation(1, truth), AgentSettings{}, belief);
    const RunResult plain = runSingleAgent(truth, plainAgent, AgentSettings{});
    return sameRun(fault, plain) && !faultAgent.state.activationHook;
}

bool emptyC0(const WorldState& truth, std::optional<WorldState> belief) {
    NaiveAgent faultAgent(makeObservation(1, truth), AgentSettings{}, belief);
    const FaultRunResult fault =
        runWithFaults(truth, faultAgent, AgentSettings{}, {});
    NaiveAgent plainAgent(makeObservation(1, truth), AgentSettings{}, belief);
    const RunResult plain = runSingleAgent(truth, plainAgent, AgentSettings{});
    return sameRun(fault, plain) && !faultAgent.state.activationHook;
}

void checkFr4(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    check(counter, isValidWorld(demo) && isValidWorld(d2), "FR4: D, D2 valid");
    check(counter, emptyC1(demo, std::nullopt),
          "FR4: empty list gives runSingleAgent for C1 on D");
    check(counter, emptyC1(d2, demo),
          "FR4: empty list gives runSingleAgent for C1 on D2, belief D");
    check(counter, emptyC0(demo, std::nullopt),
          "FR4: empty list gives runSingleAgent for C0 on D");
    check(counter, emptyC0(d2, demo),
          "FR4: empty list gives runSingleAgent for C0 on D2, belief D");
}

void checkIeT1(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    SingleAgent first(makeObservation(1, demo), AgentSettings{});
    const RunResult one = runSingleAgent(demo, first, AgentSettings{});
    bool ok = linesEqual(c1Demo1Lines(), one.lines, *counter.out);
    check(counter, ok && one.evaluations == 19,
          "IE-T1: demonstration 1 without instrumentation gives C1_1 (19)");
    SingleAgent second(makeObservation(1, d2), AgentSettings{}, demo);
    const RunResult two = runSingleAgent(d2, second, AgentSettings{});
    ok = linesEqual(c1Demo2Lines(), two.lines, *counter.out);
    check(counter, ok && two.evaluations == 414,
          "IE-T1: demonstration 2 without instrumentation gives C1_2 (414)");
}

}  // namespace

void runFaultRunTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Fault run ---\n";
    }
    checkFr1(counter);
    checkFr4(counter);
    checkIeT1(counter);
}

}  // namespace aih
