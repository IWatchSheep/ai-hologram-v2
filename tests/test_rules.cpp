// Purpose: Tests for applyAction and outcomeName (R1 to
// R11 and N2, plan Section 6.5).
// Introduced in: Stage 1b (rebuild)
// Architecture: Every check uses the OPEN layout with the
// agent and object placed by the shared at() helper, so a
// failure points at the one movement rule that changed.
// Expected values come from checks.py, the reference mirror.

#include "test_layouts.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

// One R1 check: from OPEN's (4,4), one direction moves the
// agent to the expected cell and leaves the object at (7,7).
void checkOneMove(TestCounter& counter, const std::string& description,
                  Action action, Position expectedAgent) {
    const WorldState start = at(openLayout(), Position{4, 4}, Position{7, 7});
    const ActionResult result = applyAction(start, action);
    const bool passed = result.outcome == MoveOutcome::Moved &&
                        result.state.agent == expectedAgent &&
                        result.state.object == Position{7, 7};
    check(counter, passed, description);
}

void checkFirstMoves(TestCounter& counter) {
    checkOneMove(counter, "R1a: North from (4,4) moves agent to (4,3)",
                 Action::North, Position{4, 3});
    checkOneMove(counter, "R1b: East from (4,4) moves agent to (5,4)",
                 Action::East, Position{5, 4});
    checkOneMove(counter, "R1c: South from (4,4) moves agent to (4,5)",
                 Action::South, Position{4, 5});
    checkOneMove(counter, "R1d: West from (4,4) moves agent to (3,4)",
                 Action::West, Position{3, 4});
}

void checkBlockedAndPushed(TestCounter& counter) {
    const WorldState wall = at(openLayout(), Position{1, 1}, Position{7, 7});
    const ActionResult blocked = applyAction(wall, Action::North);
    check(counter,
          blocked.outcome == MoveOutcome::Blocked && blocked.state == wall,
          "R2: North into the border wall is Blocked and unchanged");
    const WorldState push = at(openLayout(), Position{4, 4}, Position{5, 4});
    const ActionResult pushed = applyAction(push, Action::East);
    check(counter,
          pushed.outcome == MoveOutcome::Pushed &&
              pushed.state.agent == Position{5, 4} &&
              pushed.state.object == Position{6, 4},
          "R3: East pushes the object from (5,4) to (6,4)");
    const WorldState stuck = at(openLayout(), Position{7, 4}, Position{8, 4});
    const ActionResult stuckResult = applyAction(stuck, Action::East);
    check(counter,
          stuckResult.outcome == MoveOutcome::Blocked &&
              stuckResult.state.agent == Position{7, 4} &&
              stuckResult.state.object == Position{8, 4},
          "R4: East push into the border wall is Blocked and unchanged");
}

void checkGoalAndPulling(TestCounter& counter) {
    const WorldState goalPush =
        at(openLayout(), Position{3, 8}, Position{2, 8});
    const ActionResult goalResult = applyAction(goalPush, Action::West);
    check(counter,
          goalResult.outcome == MoveOutcome::Pushed &&
              isGoalReached(goalResult.state),
          "R5: West push onto the destination reaches the goal");
    const WorldState stroll = at(openLayout(), Position{2, 7}, Position{5, 5});
    const ActionResult first = applyAction(stroll, Action::West);
    const ActionResult second = applyAction(first.state, Action::South);
    check(counter,
          second.outcome == MoveOutcome::Moved &&
              second.state.agent == Position{1, 8} &&
              !isGoalReached(second.state),
          "R6: West then South puts agent on destination, goal not reached");
    const WorldState pull = at(openLayout(), Position{4, 4}, Position{5, 4});
    const ActionResult pulled = applyAction(pull, Action::West);
    check(counter,
          pulled.outcome == MoveOutcome::Moved &&
              pulled.state.object == Position{5, 4},
          "R7: West away from the object moves only the agent (no pulling)");
}

void checkWallsAndInput(TestCounter& counter) {
    const WorldState open = at(openLayout(), Position{4, 4}, Position{7, 7});
    const ActionResult moved = applyAction(open, Action::North);
    check(counter, moved.state.walls == open.walls,
          "R8a: walls are unchanged after a move");
    const WorldState push = at(openLayout(), Position{4, 4}, Position{5, 4});
    const ActionResult pushed = applyAction(push, Action::East);
    check(counter, pushed.state.walls == open.walls,
          "R8b: walls are unchanged after a push");
    const WorldState stuck = at(openLayout(), Position{7, 4}, Position{8, 4});
    const ActionResult blocked = applyAction(stuck, Action::East);
    check(counter, blocked.state.walls == open.walls,
          "R8c: walls are unchanged after a blocked push");
    const WorldState before = push;
    applyAction(push, Action::East);
    check(counter, push == before,
          "R9: applyAction does not change its input state");
}

void checkAfterGoalAndInvalid(TestCounter& counter) {
    const WorldState goal = at(openLayout(), Position{2, 7}, Position{1, 8});
    const ActionResult after = applyAction(goal, Action::East);
    check(counter,
          isGoalReached(goal) && after.outcome == MoveOutcome::Moved &&
              after.state.agent == Position{3, 7} &&
              after.state.object == Position{1, 8},
          "R10: after the goal, East still moves and the object stays");
    const WorldState outside =
        at(openLayout(), Position{10, 4}, Position{7, 7});
    const ActionResult badWest = applyAction(outside, Action::West);
    check(counter,
          badWest.outcome == MoveOutcome::InvalidState &&
              badWest.state == outside,
          "R11a: agent at (10,4) gives InvalidState and no change");
    const WorldState onWall = at(openLayout(), Position{0, 4}, Position{7, 7});
    const ActionResult badEast = applyAction(onWall, Action::East);
    check(
        counter,
        badEast.outcome == MoveOutcome::InvalidState && badEast.state == onWall,
        "R11b: agent at (0,4) on a wall gives InvalidState and no change");
}

}  // namespace

void runRulesTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Rules ---\n";
    }
    checkFirstMoves(counter);
    checkBlockedAndPushed(counter);
    checkGoalAndPulling(counter);
    checkWallsAndInput(counter);
    checkAfterGoalAndInvalid(counter);
    check(counter,
          outcomeName(MoveOutcome::Moved) == "Moved" &&
              outcomeName(MoveOutcome::Pushed) == "Pushed" &&
              outcomeName(MoveOutcome::Blocked) == "Blocked" &&
              outcomeName(MoveOutcome::InvalidState) == "InvalidState",
          "N2: outcomeName gives Moved, Pushed, Blocked, InvalidState");
}

}  // namespace aih
