// Purpose: More tests for the shadow computations of plan
// 3.9: the F3 and F4 cases of SH1.
// Introduced in: Stage 4e (rebuild)
// Architecture: Expected values are copied from checks.py
// check_shadow. A shadow is the same computation repeated
// with the fault undone, to see whether the fault could
// have made a difference. For F4 the shadow is a whole map:
// the belief analysed without the fault's "not dead"
// override. Shadow work never uses a budget and is never
// seen by an agent.

#include <cstddef>
#include <optional>
#include <vector>

#include "agent/agent_model.h"
#include "agent/dead_squares.h"
#include "agent/heuristic.h"
#include "agent_test_support.h"
#include "experiment/fault_event.h"
#include "experiment/fault_model.h"
#include "experiment/scorer_types.h"
#include "experiment/shadow.h"
#include "fault_test_material.h"
#include "model/prediction.h"
#include "model/world_model.h"
#include "stage4_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

// The prediction one model makes, as one Prediction value.
Prediction predictionOf(const ActionModel& model, const WorldState& state,
                        Action action) {
    const ActionResult result = model(state, action);
    return Prediction{action, result.outcome, result.state};
}

void checkF3(TestCounter& counter) {
    const WorldState belief = demoWorld();
    check(counter, isValidWorld(belief), "SH1-F3a: D valid");
    FaultEvent event = handBuiltEvent("t", 1, FaultType::F3);
    event.action = Action::East;
    event.rotation = Rotation::Right;
    FaultedActions table;
    table.faulted[static_cast<std::size_t>(actionIndex(Action::East))] = true;
    table.rotation[static_cast<std::size_t>(actionIndex(Action::East))] =
        Rotation::Right;
    const ActionModel faulty = makeFaultyModel(table);
    AgentActionModel agentModel(faulty);
    ScoredView view;
    view.belief = &belief;
    view.model = &agentModel;
    const std::vector<Position> noCells;
    view.notDeadOverrides = &noCells;
    view.deadOverrides = &noCells;
    WorldState state = belief;
    state.agent = Position{4, 3};
    check(counter, isValidWorld(state), "SH1-F3b: the state is valid");
    const ActionModel standard = [](const WorldState& given, Action action) {
        return applyAction(given, action);
    };
    bool ok = evaluationEffective(event, state, Action::East, faulty,
                                  predictionOf(faulty, state, Action::East),
                                  view, belief, belief.object);
    ok = ok && !evaluationEffective(event, state, Action::East, standard,
                                    predictionOf(standard, state, Action::East),
                                    view, belief, belief.object);
    ok = ok && !evaluationEffective(event, state, Action::West, faulty,
                                    predictionOf(faulty, state, Action::West),
                                    view, belief, belief.object);
    check(counter, ok,
          "SH1-F3c: faulted East effective, standard and West not");
}

void checkF4(TestCounter& counter) {
    const WorldState belief = demoWorld();
    check(counter, isValidWorld(belief), "SH1-F4a: D valid");
    FaultEvent event = handBuiltEvent("t", 1, FaultType::F4);
    event.hasCell = true;
    event.cell = Position{1, 1};
    const std::vector<Position> notDead = {Position{1, 1}};
    const std::vector<Position> noCells;
    ScoredView view;
    view.belief = &belief;
    view.notDeadOverrides = &notDead;
    view.deadOverrides = &noCells;
    const std::optional<DeadSquareMap> shadow = shadowMap(event, view, belief);
    FreeComputationCounts counts;
    const DeadSquareMap real =
        computeDeadSquares(belief, notDead, noCells, counts);
    bool ok = shadow.has_value() && shadow->dead[1][1] && !real.dead[1][1];
    ok = ok && shadow->dead[2][2] == real.dead[2][2];
    check(counter, ok, "SH1-F4b: the shadow map without the override");
    WorldState state = belief;
    state.agent = Position{3, 3};
    state.object = Position{1, 1};
    check(counter, isValidWorld(state), "SH1-F4c: the state is valid");
    bool values = heuristic(state, real, counts) == 12;
    values = values && heuristic(state, *shadow, counts) == kInfiniteHeuristic;
    check(counter, values,
          "SH1-F4d: heuristic 12 against the real map, "
          "infinite against the shadow");
}

}  // namespace

void runShadowMoreTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Shadow more ---\n";
    }
    checkF3(counter);
    checkF4(counter);
}

}  // namespace aih
