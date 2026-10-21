// Purpose: Tests for the shadow computations of plan 3.9:
// the F1 and F2 cases of SH1.
// Introduced in: Stage 4e (rebuild)
// Architecture: Expected values are copied from checks.py
// check_shadow. A shadow is the same computation repeated
// with the fault undone, to see whether the fault could
// have made a difference. Shadow work never uses a budget
// and is never seen by an agent: these tests call the pure
// shadow functions directly, with a fixed read-only view.

#include <vector>

#include "agent/agent_model.h"
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

// A fixed read-only view over the given belief and model,
// with the given override lists, as checks.py FixedView.
ScoredView fixedView(const WorldState& belief, const AgentActionModel& model,
                     const std::vector<Position>& notDead,
                     const std::vector<Position>& dead) {
    ScoredView view;
    view.belief = &belief;
    view.model = &model;
    view.notDeadOverrides = &notDead;
    view.deadOverrides = &dead;
    return view;
}

void checkF1(TestCounter& counter) {
    WorldState belief = demoWorld();
    const WorldState truth = withWallAt(belief, Position{4, 3}, true);
    check(counter, isValidWorld(belief) && isValidWorld(truth),
          "SH1-F1a: D and D2 valid");
    FaultEvent event = handBuiltEvent("t", 1, FaultType::F1);
    event.direction = F1Direction::BlockedToFree;
    event.hasCell = true;
    event.cell = Position{4, 3};
    AgentActionModel agentModel;
    const std::vector<Position> noCells;
    const ScoredView view = fixedView(belief, agentModel, noCells, noCells);
    const ActionModel model = agentModel.current();
    bool ok = evaluationEffective(event, belief, Action::East, model,
                                  predictionOf(model, belief, Action::East),
                                  view, truth, truth.object);
    ok = ok && !evaluationEffective(event, belief, Action::North, model,
                                    predictionOf(model, belief, Action::North),
                                    view, truth, truth.object);
    check(counter, ok, "SH1-F1b: East effective, North not");
}

void checkF2(TestCounter& counter) {
    WorldState belief = demoWorld();
    belief.object = Position{5, 2};
    const WorldState truth = demoWorld();
    check(counter, isValidWorld(belief) && isValidWorld(truth),
          "SH1-F2a: belief with object (5, 2) and D valid");
    FaultEvent event = handBuiltEvent("t", 1, FaultType::F2);
    event.hasCell = true;
    AgentActionModel agentModel;
    const std::vector<Position> noCells;
    const ScoredView view = fixedView(belief, agentModel, noCells, noCells);
    const ActionModel model = agentModel.current();
    WorldState state = belief;
    state.agent = Position{4, 2};
    check(counter, isValidWorld(state), "SH1-F2b: the state is valid");
    bool ok = evaluationEffective(event, state, Action::East, model,
                                  predictionOf(model, state, Action::East),
                                  view, truth, Position{5, 3});
    ok = ok && !evaluationEffective(event, state, Action::West, model,
                                    predictionOf(model, state, Action::West),
                                    view, truth, Position{5, 3});
    check(counter, ok, "SH1-F2c: East effective, West not");
    WorldState odd = belief;
    odd.agent = Position{5, 3};
    check(counter, isValidWorld(odd), "SH1-F2d: the odd state is valid");
    check(counter,
          evaluationEffective(event, odd, Action::North, model,
                              predictionOf(model, odd, Action::North), view,
                              truth, Position{5, 3}),
          "SH1-F2e: an invalid shadow counts as effective");
}

}  // namespace

void runShadowTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Shadow ---\n";
    }
    checkF1(counter);
    checkF2(counter);
}

}  // namespace aih
