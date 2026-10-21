// Purpose: Tests for the agent's action model (AM1 to AM5,
// plan Section 6.3), including the snapshot rule and H2.
// Introduced in: Stage 3c (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld. Expected
// values come from checks.py (check_agent_model, push_north),
// and each description starts with its check id.

#include "agent/agent_model.h"
#include "agent_test_support.h"
#include "model/world_model.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

// The wrong base model of checks.py: an East push moves the
// object North instead of East; everything else is standard.
ActionResult pushNorth(const WorldState& state, Action action) {
    const ActionResult standard = applyAction(state, action);
    if (action != Action::East) {
        return standard;
    }
    if (standard.outcome != MoveOutcome::Pushed) {
        return standard;
    }
    ActionResult wrong = standard;
    wrong.state.object = moved(state.object, Action::North);
    return wrong;
}

// A model that always blocks, leaving the state unchanged.
ActionResult alwaysBlocked(const WorldState& state, Action action) {
    (void)action;
    return ActionResult{state, MoveOutcome::Blocked};
}

void checkAm1Am2(TestCounter& counter) {
    WorldState state = demoWorld();
    state = withAgentAt(state, Position{4, 3});
    const AgentActionModel standard;
    const ActionResult pushed = standard.current()(state, Action::East);
    check(counter, isValidWorld(state) && pushed.state.object == Position{6, 3},
          "AM1: AgentActionModel() is standard (East push to (6, 3))");
    const AgentActionModel wrong(pushNorth);
    const ActionResult north = wrong.current()(state, Action::East);
    const bool ok = !wrong.isRestored(Action::East);
    check(counter, ok && north.state.object == Position{5, 2},
          "AM2: push-north base, not restored: object to (5, 2)");
}

void checkAm3Am4(TestCounter& counter) {
    WorldState state = demoWorld();
    state = withAgentAt(state, Position{4, 3});
    AgentActionModel model(pushNorth);
    const ActionModel snapshot = model.current();
    model.restore(Action::East);
    bool ok = model.isRestored(Action::East);
    ok = ok &&
         model.current()(state, Action::East).state.object == Position{6, 3};
    ok = ok && model.base()(state, Action::East).state.object == Position{5, 2};
    ok = ok && snapshot(state, Action::East).state.object == Position{5, 2};
    check(counter, isValidWorld(state) && ok,
          "AM3: restore(East) standard; base and old snapshot still push "
          "north");
    model.unrestore(Action::East);
    const bool back = !model.isRestored(Action::East);
    const ActionResult north = model.current()(state, Action::East);
    check(counter, back && north.state.object == Position{5, 2},
          "AM4: unrestore(East): back to the base model");
}

void checkAm5(TestCounter& counter) {
    WorldState state = demoWorld();
    state = withAgentAt(state, Position{4, 3});
    AgentActionModel model(pushNorth);
    model.restore(Action::East);
    model.setBase(alwaysBlocked);
    bool ok = model.isRestored(Action::East);
    ok = ok &&
         model.current()(state, Action::East).state.object == Position{6, 3};
    ok = ok &&
         model.current()(state, Action::West).outcome == MoveOutcome::Blocked;
    model.setBase(ActionModel{});
    ok = ok &&
         model.current()(state, Action::West).outcome == MoveOutcome::Moved;
    check(counter, isValidWorld(state) && ok,
          "AM5: setBase (H2) keeps East mark; empty model is standard "
          "(West Moved)");
}

}  // namespace

void runAgentModelTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Agent model ---\n";
    }
    checkAm1Am2(counter);
    checkAm3Am4(counter);
    checkAm5(counter);
}

}  // namespace aih
