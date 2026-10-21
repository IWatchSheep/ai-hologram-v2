// Purpose: Tests for applying F1 and F2 faults (FI1, FI2;
// plan Section 6.2), with A3's refusal reasons.
// Introduced in: Stage 4c (rebuild)
// Architecture: Expected values are copied from checks.py
// check_apply. Applying a fault changes only the reasoner's
// belief here; the world passed in is never changed. Every
// fixture is a valid world, checked with isValidWorld.

#include <vector>

#include "agent/observation.h"
#include "agent/single_agent.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
#include "experiment/fault_apply.h"
#include "experiment/fault_event.h"
#include "fault_test_material.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

// Applies one hand-built event to one fresh reasoner's
// parts, with a fresh faulted-actions table, as checks.py
// apply() does.
ApplyOutcome applyTo(SingleAgent& agent, const FaultEvent& event) {
    FaultedActions faulted;
    const std::vector<Observation> noHistory;
    return tryApplyFault(event, partsOf(agent.state), faulted, noHistory);
}

FaultEvent f1At(Position cell) {
    FaultEvent event = handBuiltEvent("t", 1, FaultType::F1);
    event.hasCell = true;
    event.cell = cell;
    return event;
}

void checkFi1(TestCounter& counter) {
    const WorldState demo = demoWorld();
    check(counter, isValidWorld(demo), "FI1: D valid");
    {
        SingleAgent agent(makeObservation(1, demo), AgentSettings{});
        const ApplyOutcome outcome = applyTo(agent, f1At(Position{3, 3}));
        check(
            counter,
            !outcome.applied &&
                outcome.reason == "the wall would be under the believed agent",
            "FI1: F1 free->blocked at (3, 3) refused (believed agent)");
    }
    {
        SingleAgent agent(makeObservation(1, demo), AgentSettings{});
        const ApplyOutcome outcome = applyTo(agent, f1At(Position{5, 3}));
        check(
            counter,
            !outcome.applied &&
                outcome.reason == "the wall would be under the believed object",
            "FI1: F1 free->blocked at (5, 3) refused (believed object)");
    }
}

void checkFi1Applied(TestCounter& counter) {
    const WorldState demo = demoWorld();
    {
        SingleAgent agent(makeObservation(1, demo), AgentSettings{});
        const ApplyOutcome outcome = applyTo(agent, f1At(Position{6, 5}));
        check(counter, outcome.applied && agent.state.belief.walls[5][6],
              "FI1: F1 free->blocked at (6, 5) applied");
    }
    {
        SingleAgent agent(makeObservation(1, demo), AgentSettings{});
        FaultEvent event = f1At(Position{4, 6});
        event.direction = F1Direction::BlockedToFree;
        const ApplyOutcome outcome = applyTo(agent, event);
        check(counter, outcome.applied && !agent.state.belief.walls[6][4],
              "FI1: F1 blocked->free at (4, 6) applied");
    }
    {
        SingleAgent agent(makeObservation(1, demo), AgentSettings{});
        FaultEvent event = handBuiltEvent("t", 1, FaultType::F1);
        const ApplyOutcome outcome = applyTo(agent, event);
        check(counter,
              !outcome.applied &&
                  outcome.reason == "no cell of the required kind",
              "FI1: F1 with no cell refused");
    }
}

FaultEvent f2Event() {
    FaultEvent event = handBuiltEvent("t", 1, FaultType::F2);
    event.order = {Action::North, Action::East, Action::South, Action::West};
    return event;
}

void checkFi2(TestCounter& counter) {
    const WorldState demo = demoWorld();
    {
        SingleAgent agent(makeObservation(1, demo), AgentSettings{});
        const ApplyOutcome outcome = applyTo(agent, f2Event());
        check(counter,
              outcome.applied && agent.state.belief.object == Position{5, 2},
              "FI2: F2 N,E,S,W moves the believed object to (5, 2)");
    }
    {
        SingleAgent agent(makeObservation(1, demo), AgentSettings{});
        agent.state.belief.object = Position{1, 1};
        agent.state.belief.walls[1][2] = true;
        agent.state.belief.walls[2][1] = true;
        check(counter, isValidWorld(agent.state.belief),
              "FI2: the hand-built boxed belief is valid");
        const ApplyOutcome outcome = applyTo(agent, f2Event());
        check(counter,
              !outcome.applied &&
                  outcome.reason == "no direction passes the test",
              "FI2: F2 boxed in is refused");
    }
    {
        SingleAgent agent(makeObservation(1, demo), AgentSettings{});
        agent.state.belief.agent = Position{5, 2};
        const ApplyOutcome outcome = applyTo(agent, f2Event());
        check(counter,
              outcome.applied && agent.state.belief.object == Position{6, 3},
              "FI2: F2 with the agent believed at (5, 2) gives (6, 3)");
    }
}

}  // namespace

void runFaultApplyTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Fault apply ---\n";
    }
    checkFi1(counter);
    checkFi1Applied(counter);
    checkFi2(counter);
}

}  // namespace aih
