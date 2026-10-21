// Purpose: Tests for Environment (E1 to E4, plan 6.6).
// Introduced in: Stage 1b (rebuild)
// Architecture: Each check starts from the OPEN layout.
// E3 compares Environment, step by step, with applyAction
// applied by hand, showing the true state and predictions
// follow the same rules. Expected values are from checks.py.

#include <vector>

#include "test_layouts.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/environment.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"
#include "world/world_text.h"

namespace aih {
namespace {

// Returns the parsed OPEN world, or an empty state on failure.
WorldState parsedOpen() {
    const ParseResult result = parseWorld(openLayout());
    if (result.world.has_value()) {
        return result.world.value();
    }
    return WorldState{};
}

void checkNewAndNorth(TestCounter& counter) {
    const WorldState open = parsedOpen();
    Environment fresh(open);
    check(counter, fresh.state() == open && fresh.stepCount() == 0,
          "E1: new Environment from OPEN has OPEN state and step count 0");
    Environment walker(open);
    walker.step(Action::North);
    walker.step(Action::North);
    walker.step(Action::North);
    walker.step(Action::North);
    check(counter,
          walker.stepCount() == 4 && walker.state().agent == Position{4, 1},
          "E2: North four times gives step count 4 and agent at (4,1)");
}

void checkMatchesByHand(TestCounter& counter) {
    const WorldState open = parsedOpen();
    const std::vector<Action> actions = {
        Action::East,  Action::East, Action::South, Action::South,
        Action::South, Action::East, Action::West,  Action::North};
    Environment environment(open);
    WorldState byHand = open;
    for (const Action action : actions) {
        environment.step(action);
        byHand = applyAction(byHand, action).state;
    }
    check(counter,
          environment.state() == byHand && environment.stepCount() == 8,
          "E3: Environment matches applyAction by hand over 8 steps");
}

void checkInvalidStep(TestCounter& counter) {
    const WorldState bad = at(openLayout(), Position{0, 4}, Position{7, 7});
    Environment environment(bad);
    const ActionResult result = environment.step(Action::East);
    check(counter,
          result.outcome == MoveOutcome::InvalidState &&
              environment.state() == bad && environment.stepCount() == 1,
          "E4: step East from a wall gives InvalidState, unchanged, count 1");
}

}  // namespace

void runEnvironmentTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Environment ---\n";
    }
    checkNewAndNorth(counter);
    checkMatchesByHand(counter);
    checkInvalidStep(counter);
}

}  // namespace aih
