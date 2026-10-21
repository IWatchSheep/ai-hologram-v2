// Purpose: Tests for applying F3 and F4 faults (FI3, FI4;
// plan Section 6.2): the faulty model and the "not dead"
// override.
// Introduced in: Stage 4c (rebuild)
// Architecture: Expected values are copied from checks.py
// check_f3_f4. An F3 fault is a process fault: it installs
// a faulty action model in the reasoner (never in the
// world), and restoring one action or replacing the base
// model must keep the other faults and marks as the
// reference says (IE-1 H2).

#include "agent/dead_squares.h"
#include "agent/observation.h"
#include "agent/single_agent.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
#include "experiment/fault_apply.h"
#include "experiment/fault_event.h"
#include "experiment/fault_model.h"
#include "fault_test_material.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

FaultEvent f3Event(Action action, Rotation rotation) {
    FaultEvent event = handBuiltEvent("t", 1, FaultType::F3);
    event.action = action;
    event.rotation = rotation;
    return event;
}

int deadCount(const DeadSquareMap& map) {
    int count = 0;
    for (const auto& row : map.dead) {
        for (const bool dead : row) {
            if (dead) {
                count = count + 1;
            }
        }
    }
    return count;
}

void checkFi3(TestCounter& counter) {
    const WorldState demo = demoWorld();
    SingleAgent agent(makeObservation(1, demo), AgentSettings{});
    FaultedActions faulted;
    const std::vector<Observation> noHistory;
    const ApplyOutcome first =
        tryApplyFault(f3Event(Action::East, Rotation::Right),
                      partsOf(agent.state), faulted, noHistory);
    WorldState input = withAgentAt(demo, Position{4, 3});
    check(counter, isValidWorld(input), "FI3: the hand input is valid");
    const ActionResult direct = makeFaultyModel(faulted)(input, Action::East);
    const ActionResult result =
        agent.state.agentModel.current()(input, Action::East);
    bool ok = first.applied && result.outcome == MoveOutcome::Pushed;
    ok = ok && result.state.agent == Position{5, 3};
    ok = ok && result.state.object == Position{5, 4};
    ok = ok && direct.state.object == Position{5, 4};
    ok = ok && applyAction(input, Action::East).state.object == Position{6, 3};
    check(counter, ok,
          "FI3: East faulted right predicts (5, 4) (standard (6, 3))");
    WorldState walled = input;
    walled.walls[4][5] = true;
    const ActionResult blocked =
        agent.state.agentModel.current()(walled, Action::East);
    check(counter,
          blocked.outcome == MoveOutcome::Blocked && blocked.state == walled,
          "FI3: faulty East push is Blocked against a wall at (5, 4)");
    const ActionResult west =
        agent.state.agentModel.current()(input, Action::West);
    check(counter,
          west.outcome == MoveOutcome::Moved &&
              west.state.agent == Position{3, 3},
          "FI3: West stays standard under an East fault");
}

void checkFi3More(TestCounter& counter) {
    const WorldState demo = demoWorld();
    SingleAgent agent(makeObservation(1, demo), AgentSettings{});
    FaultedActions faulted;
    const std::vector<Observation> noHistory;
    tryApplyFault(f3Event(Action::East, Rotation::Right), partsOf(agent.state),
                  faulted, noHistory);
    const WorldState input = withAgentAt(demo, Position{4, 3});
    const ApplyOutcome second =
        tryApplyFault(f3Event(Action::North, Rotation::Left),
                      partsOf(agent.state), faulted, noHistory);
    const WorldState northInput = withAgentAt(demo, Position{5, 4});
    const ActionResult north =
        agent.state.agentModel.current()(northInput, Action::North);
    bool ok = second.applied;
    ok = ok &&
         agent.state.agentModel.current()(input, Action::East).state.object ==
             Position{5, 4};
    ok = ok && north.outcome == MoveOutcome::Pushed;
    ok = ok && north.state.agent == Position{5, 3};
    ok = ok && north.state.object == Position{4, 3};
    check(counter, ok, "FI3: a second F3 North left keeps East faulted");
    agent.state.agentModel.restore(Action::East);
    ok = agent.state.agentModel.current()(input, Action::East).state.object ==
         Position{6, 3};
    ok = ok && agent.state.agentModel.current()(northInput, Action::North)
                       .state.object == Position{4, 3};
    check(counter, ok, "FI3: restoring East leaves North faulted");
    agent.state.agentModel.setBase(makeFaultyModel(faulted));
    ok = agent.state.agentModel.isRestored(Action::East);
    ok = ok &&
         agent.state.agentModel.current()(input, Action::East).state.object ==
             Position{6, 3};
    ok = ok && agent.state.agentModel.current()(northInput, Action::North)
                       .state.object == Position{4, 3};
    check(counter, ok, "FI3: setBase keeps the restored mark (H2)");
}

void checkFi4(TestCounter& counter) {
    const WorldState demo = demoWorld();
    SingleAgent agent(makeObservation(1, demo), AgentSettings{});
    const DeadSquareMap& before = agent.state.deadCache.get(
        agent.state.belief, agent.state.notDeadOverrides,
        agent.state.deadOverrides, agent.state.freeCounts);
    const int beforeCount = deadCount(before);
    FaultEvent event = handBuiltEvent("t", 1, FaultType::F4);
    event.hasCell = true;
    event.cell = Position{1, 1};
    FaultedActions faulted;
    const std::vector<Observation> noHistory;
    const ApplyOutcome outcome =
        tryApplyFault(event, partsOf(agent.state), faulted, noHistory);
    const DeadSquareMap& after = agent.state.deadCache.get(
        agent.state.belief, agent.state.notDeadOverrides,
        agent.state.deadOverrides, agent.state.freeCounts);
    bool ok = outcome.applied && !after.dead[1][1];
    ok = ok && beforeCount == 22 && deadCount(after) == 21;
    check(counter, ok,
          "FI4: F4 at (1, 1): not dead, 21 dead cells (22 before)");
    agent.state.deadOverrides.push_back(Position{1, 1});
    const DeadSquareMap& corrected = agent.state.deadCache.get(
        agent.state.belief, agent.state.notDeadOverrides,
        agent.state.deadOverrides, agent.state.freeCounts);
    check(counter, corrected.dead[1][1],
          "FI4: a later dead correction makes (1, 1) dead again");
    FaultEvent noCell = handBuiltEvent("t", 1, FaultType::F4);
    FaultedActions fresh;
    const ApplyOutcome refused =
        tryApplyFault(noCell, partsOf(agent.state), fresh, noHistory);
    check(counter,
          !refused.applied &&
              refused.reason == "the layout has no true dead square",
          "FI4: F4 with no cell refused");
}

}  // namespace

void runFaultApplyMoreTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Fault apply more ---\n";
    }
    checkFi3(counter);
    checkFi3More(counter);
    checkFi4(counter);
}

}  // namespace aih
