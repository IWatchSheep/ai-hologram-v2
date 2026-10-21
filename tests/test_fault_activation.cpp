// Purpose: Tests for the activation records (FI7, FI8;
// plan Section 6.2): deferral of F5, runs that end too
// soon, and the five reporting texts.
// Introduced in: Stage 4c (rebuild)
// Architecture: Expected values are copied from checks.py
// check_activation. A refused F5 is retried later because
// the object may move and make it applicable (A4); other
// refused faults are not retried, because waiting alone
// does not change their reason.

#include <vector>

#include "agent/observation.h"
#include "agent/single_agent.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
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

// One demonstration run with faults, as checks.py
// demo_run() builds it: a fresh C1 on the run's layout.
FaultRunResult demoRun(const FaultTestMaterial& material, int index) {
    const WorldState& world = material.layouts[index];
    SingleAgent agent(makeObservation(1, world), AgentSettings{});
    return runWithFaults(world, agent, AgentSettings{}, material.events[index]);
}

void checkFi7(TestCounter& counter) {
    const FaultTestMaterial material = demonstrationFaultMaterial();
    const FaultRunResult deferred = demoRun(material, 6);
    const ActivationRecord first =
        findActivation(deferred.activations, "500003-1-1");
    bool ok = first.status == ActivationStatus::Applied;
    ok = ok && first.applicationStep == 9 && activationDelay(first) == 8;
    ok = ok && reportedAs(first) == "Activated after deferral";
    check(counter, ok, "FI7: 500003-1-1 applied at 9, delay 8");
    const FaultRunResult pending = demoRun(material, 2);
    const ActivationRecord second =
        findActivation(pending.activations, "500001-1-3");
    ok = second.status == ActivationStatus::Pending;
    ok = ok && second.reason == "the object has not moved yet";
    ok =
        ok && reportedAs(second) == "Not activated: deferred, never applicable";
    check(counter, ok, "FI7: 500001-1-3 pending, the object has not moved yet");
}

FaultEvent orderEvent(int step) {
    FaultEvent event = handBuiltEvent("t", step, FaultType::F2);
    event.order = {Action::North, Action::East, Action::South, Action::West};
    return event;
}

void checkFi8(TestCounter& counter) {
    const WorldState demo = demoWorld();
    {
        SingleAgent agent(makeObservation(1, demo), AgentSettings{});
        const FaultRunResult run =
            runWithFaults(demo, agent, AgentSettings{},
                          std::vector<FaultEvent>{orderEvent(10)});
        bool ok = run.run.outcome == "Success" && run.run.worldSteps == 4;
        ok = ok && run.activations.size() == 1;
        if (run.activations.size() == 1) {
            ok = ok && reportedAs(run.activations[0]) ==
                           "Not activated: run ended before the scheduled step";
        }
        check(counter, ok, "FI8: a step-10 event ends before its step");
    }
    {
        SingleAgent agent(makeObservation(1, demo), AgentSettings{});
        FaultEvent event = handBuiltEvent("t", 1, FaultType::F1);
        event.hasCell = true;
        event.cell = Position{3, 3};
        const FaultRunResult run = runWithFaults(
            demo, agent, AgentSettings{}, std::vector<FaultEvent>{event});
        bool ok = run.run.outcome == "Success" && run.run.worldSteps == 4;
        ok = ok && run.run.evaluations == 19 && run.activations.size() == 1;
        if (run.activations.size() == 1) {
            ok =
                ok && reportedAs(run.activations[0]) ==
                          "Not activated: not applicable at the scheduled step";
            ok = ok && run.activations[0].reason ==
                           "the wall would be under the believed agent";
        }
        check(
            counter, ok,
            "FI8: F1 at (3, 3) not applicable, run unchanged (19 evaluations)");
    }
}

void checkFi8Texts(TestCounter& counter) {
    ActivationRecord record;
    record.eventId = "t";
    record.scheduledStep = 5;
    record.status = ActivationStatus::Applied;
    record.applicationStep = 5;
    bool ok = activationDelay(record) == 0;
    ok = ok && reportedAs(record) == "Activated at its scheduled step";
    record.applicationStep = 8;
    ok = ok && activationDelay(record) == 3;
    ok = ok && reportedAs(record) == "Activated after deferral";
    ActivationRecord other;
    other.scheduledStep = 0;
    ok = ok && activationDelay(other) == 0;
    ok = ok && reportedAs(other) ==
                   "Not activated: run ended before the scheduled step";
    other.status = ActivationStatus::NotApplicable;
    ok = ok && reportedAs(other) ==
                   "Not activated: not applicable at the scheduled step";
    other.status = ActivationStatus::Pending;
    ok = ok && activationDelay(other) == 0;
    ok = ok && reportedAs(other) == "Not activated: deferred, never applicable";
    check(counter, ok, "FI8: reportedAs and activationDelay for every status");
}

}  // namespace

void runFaultActivationTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Fault activation ---\n";
    }
    checkFi7(counter);
    checkFi8(counter);
    checkFi8Texts(counter);
}

}  // namespace aih
