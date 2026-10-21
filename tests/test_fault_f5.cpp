// Purpose: Tests for applying an F5 fault and for the
// predictor cache after faults (FI5, FI6; plan 6.2).
// Introduced in: Stage 4c (rebuild)
// Architecture: Expected values are copied from checks.py
// check_f5_cache. F5's previous square comes from the TRUE
// history (never the belief), while A3's two checks use
// the reasoner's own belief. An applied fault empties the
// predictor cache (D-036); a refused one leaves it.

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
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

ApplyOutcome applyWith(SingleAgent& agent, const FaultEvent& event,
                       const std::vector<Observation>& history) {
    FaultedActions faulted;
    return tryApplyFault(event, partsOf(agent.state), faulted, history);
}

FaultEvent f1At(Position cell) {
    FaultEvent event = handBuiltEvent("t", 1, FaultType::F1);
    event.hasCell = true;
    event.cell = cell;
    return event;
}

void checkFi5(TestCounter& counter) {
    const WorldState demo = demoWorld();
    check(counter, isValidWorld(demo), "FI5: D valid");
    WorldState s2 = withAgentAt(demo, Position{4, 3});
    WorldState s3 = withAgentAt(s2, Position{5, 3});
    s3.object = Position{6, 3};
    WorldState s4 = withAgentAt(s3, Position{5, 2});
    const bool statesOk =
        isValidWorld(s2) && isValidWorld(s3) && isValidWorld(s4);
    check(counter, statesOk, "FI5: the history states are valid");
    const std::vector<Observation> history = {
        makeObservation(1, demo), makeObservation(2, s2),
        makeObservation(3, s3), makeObservation(4, s4)};
    const FaultEvent f5 = handBuiltEvent("t", 1, FaultType::F5);
    {
        SingleAgent agent(makeObservation(1, demo), AgentSettings{});
        agent.state.belief = s3;
        const std::vector<Observation> firstThree(history.begin(),
                                                  history.begin() + 3);
        const ApplyOutcome outcome = applyWith(agent, f5, firstThree);
        check(counter,
              !outcome.applied &&
                  outcome.reason ==
                      "the previous square is the believed agent square",
              "FI5: F5 refused on the believed agent square");
    }
    {
        SingleAgent agent(makeObservation(1, demo), AgentSettings{});
        agent.state.belief = s4;
        const ApplyOutcome outcome = applyWith(agent, f5, history);
        check(counter,
              outcome.applied && agent.state.belief.object == Position{5, 3},
              "FI5: F5 applied after the agent moved north (object (5, 3))");
    }
}

void checkFi5More(TestCounter& counter) {
    const WorldState demo = demoWorld();
    WorldState s2 = withAgentAt(demo, Position{4, 3});
    WorldState s3 = withAgentAt(s2, Position{5, 3});
    s3.object = Position{6, 3};
    WorldState s4 = withAgentAt(s3, Position{5, 2});
    const std::vector<Observation> history = {
        makeObservation(1, demo), makeObservation(2, s2),
        makeObservation(3, s3), makeObservation(4, s4)};
    const FaultEvent f5 = handBuiltEvent("t", 1, FaultType::F5);
    {
        SingleAgent agent(makeObservation(1, demo), AgentSettings{});
        const std::vector<Observation> firstOne(history.begin(),
                                                history.begin() + 1);
        const ApplyOutcome outcome = applyWith(agent, f5, firstOne);
        check(counter,
              !outcome.applied &&
                  outcome.reason == "the object has not moved yet",
              "FI5: F5 refused when the object has not moved yet");
    }
    {
        SingleAgent agent(makeObservation(1, demo), AgentSettings{});
        WorldState belief = s4;
        belief.walls[3][5] = true;
        agent.state.belief = belief;
        const ApplyOutcome outcome = applyWith(agent, f5, history);
        check(counter,
              !outcome.applied &&
                  outcome.reason ==
                      "the previous square is believed to be a wall",
              "FI5: F5 refused on a believed wall");
    }
}

void checkFi6(TestCounter& counter) {
    const WorldState demo = demoWorld();
    SingleAgent agent(makeObservation(1, demo), AgentSettings{});
    agent.state.predictor.predict(agent.state.belief, Action::East);
    const bool before =
        agent.state.predictor.isCached(agent.state.belief, Action::East);
    FaultedActions faulted;
    const std::vector<Observation> noHistory;
    const ApplyOutcome applied = tryApplyFault(
        f1At(Position{6, 5}), partsOf(agent.state), faulted, noHistory);
    const bool cleared =
        !agent.state.predictor.isCached(agent.state.belief, Action::East);
    check(counter, before && applied.applied && cleared,
          "FI6: an applied fault empties the cache");
    agent.state.predictor.predict(agent.state.belief, Action::East);
    const bool again =
        agent.state.predictor.isCached(agent.state.belief, Action::East);
    FaultedActions fresh;
    const ApplyOutcome refused = tryApplyFault(
        f1At(Position{3, 3}), partsOf(agent.state), fresh, noHistory);
    const bool kept =
        agent.state.predictor.isCached(agent.state.belief, Action::East);
    check(counter, again && !refused.applied && kept,
          "FI6: a refused fault leaves the cache");
}

}  // namespace

void runFaultF5Tests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Fault F5 ---\n";
    }
    checkFi5(counter);
    checkFi5More(counter);
    checkFi6(counter);
}

}  // namespace aih
