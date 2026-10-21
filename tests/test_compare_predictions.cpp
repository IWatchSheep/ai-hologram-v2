// Purpose: Tests for prediction comparison and difference
// text (CM7 to CM10, plan Section 6.4).
// Introduced in: Stage 2b (rebuild)
// Architecture: Predictions are built by hand from the
// parsed OPEN layout and compared with applyAction results
// or hand-built observations. Expected values, including
// the five CM10 lines, come from checks.py (CM10_LINES).

#include <string>
#include <vector>

#include "compare/difference_text.h"
#include "compare/state_difference.h"
#include "model/prediction.h"
#include "test_layouts.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
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

void checkOutcomeCases(TestCounter& counter) {
    const WorldState base = parsedOpen();
    const Prediction blocked{Action::East, MoveOutcome::Blocked, base};
    const ActionResult moved = applyAction(base, Action::East);
    const PredictionComparison differing = comparePrediction(blocked, moved);
    check(counter, outcomesDiffer(differing) && !matches(differing),
          "CM7: Blocked prediction vs Moved result differs, not a match");
    const Prediction exact{Action::East, MoveOutcome::Moved, moved.state};
    const ActionResult observed{moved.state, MoveOutcome::Moved};
    const PredictionComparison matching = comparePrediction(exact, observed);
    check(counter, matches(matching),
          "CM8: Moved prediction with the moved state matches");
}

void checkRightOutcomeWrongState(TestCounter& counter) {
    const WorldState base = parsedOpen();
    WorldState walled = base;
    walled.walls[5][5] = true;
    const Prediction prediction{Action::East, MoveOutcome::Moved, walled};
    const ActionResult observed{base, MoveOutcome::Moved};
    const PredictionComparison comparison =
        comparePrediction(prediction, observed);
    const bool passed = !outcomesDiffer(comparison) && !matches(comparison);
    check(counter, passed,
          "CM9: equal outcomes but a wrong wall is not a match");
}

void checkDifferenceText(TestCounter& counter) {
    const WorldState base = parsedOpen();
    WorldState observed = base;
    observed.agent = Position{4, 3};
    observed.object = Position{7, 6};
    observed.destination = Position{2, 8};
    observed.walls[4][5] = true;
    observed.walls[5][4] = true;
    const std::vector<std::string> expected = {
        "  - agent: predicted (4, 4), observed (4, 3)",
        "  - object: predicted (7, 7), observed (7, 6)",
        "  - destination: predicted (1, 8), observed (2, 8)",
        "  - wall at (5, 4): predicted free, observed blocked",
        "  - wall at (4, 5): predicted free, observed blocked"};
    const std::vector<std::string> actual =
        describeDifferences(compareStates(base, observed));
    check(counter, actual == expected,
          "CM10: describeDifferences gives the five exact lines in order");
}

}  // namespace

void runComparePredictionsTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Compare predictions ---\n";
    }
    checkOutcomeCases(counter);
    checkRightOutcomeWrongState(counter);
    checkDifferenceText(counter);
}

}  // namespace aih
