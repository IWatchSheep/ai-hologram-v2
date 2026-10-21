// Purpose: Tests for WorldModel (WM1a to WM8, plan
// Section 6.2).
// Introduced in: Stage 2a (rebuild)
// Architecture: Every check starts from the parsed OPEN
// layout, held in a Belief, so a failure points at the one
// rule that changed. Expected values come from checks.py,
// the reference mirror in tools/reference/stage2/.

#include <optional>

#include "model/belief.h"
#include "model/budget_counter.h"
#include "model/prediction.h"
#include "model/world_model.h"
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

// One WM1 check: the default model's prediction for one
// action has the outcome and state of applyAction on OPEN.
void checkOneDefault(TestCounter& counter, const std::string& description,
                     Action action) {
    const WorldState open = parsedOpen();
    const Belief belief{open};
    BudgetCounter budget(1);
    const WorldModel model;
    const std::optional<Prediction> prediction =
        model.predict(belief, action, budget);
    const ActionResult expected = applyAction(open, action);
    const bool passed = prediction.has_value() &&
                        prediction.value().outcome == expected.outcome &&
                        prediction.value().state == expected.state;
    check(counter, passed, description);
}

void checkDefaultPredictions(TestCounter& counter) {
    checkOneDefault(counter,
                    "WM1a: default model North prediction matches applyAction",
                    Action::North);
    checkOneDefault(counter,
                    "WM1b: default model East prediction matches applyAction",
                    Action::East);
    checkOneDefault(counter,
                    "WM1c: default model South prediction matches applyAction",
                    Action::South);
    checkOneDefault(counter,
                    "WM1d: default model West prediction matches applyAction",
                    Action::West);
}

void checkBudgetUse(TestCounter& counter) {
    const Belief belief{parsedOpen()};
    BudgetCounter budget(5);
    const WorldModel model;
    model.predict(belief, Action::North, budget);
    model.predict(belief, Action::East, budget);
    check(counter, budget.used() == 2,
          "WM2: two predictions with BudgetCounter(5) leave used = 2");
    BudgetCounter usedUp(1);
    usedUp.tryUse();
    Belief kept{parsedOpen()};
    const WorldState before = kept.state;
    const std::optional<Prediction> refused =
        model.predict(kept, Action::East, usedUp);
    check(counter,
          !refused.has_value() && usedUp.used() == 1 && kept.state == before,
          "WM3: used-up BudgetCounter(1) gives no prediction, belief kept");
}

void checkBeliefUnchanged(TestCounter& counter) {
    Belief belief{parsedOpen()};
    const WorldState before = belief.state;
    BudgetCounter budget(1);
    const WorldModel model;
    model.predict(belief, Action::East, budget);
    check(counter, belief.state == before,
          "WM4: predict does not change its belief");
    const WorldState bad = at(openLayout(), Position{0, 4}, Position{7, 7});
    const Belief badBelief{bad};
    BudgetCounter badBudget(1);
    const std::optional<Prediction> badPrediction =
        model.predict(badBelief, Action::East, badBudget);
    const bool passed =
        badPrediction.has_value() &&
        badPrediction.value().outcome == MoveOutcome::InvalidState &&
        badPrediction.value().state == bad && badBudget.used() == 1;
    check(counter, passed,
          "WM5: agent on a wall gives InvalidState, unchanged, used 1");
}

void checkCustomModel(TestCounter& counter) {
    const WorldState open = parsedOpen();
    const Belief belief{open};
    // This supplied model always reports Blocked and never
    // moves anything, whatever the true rules would do.
    const ActionModel alwaysBlocked = [](const WorldState& state, Action) {
        return ActionResult{state, MoveOutcome::Blocked};
    };
    const WorldModel model(alwaysBlocked);
    BudgetCounter budget(1);
    const std::optional<Prediction> prediction =
        model.predict(belief, Action::East, budget);
    const bool passed = prediction.has_value() &&
                        prediction.value().outcome == MoveOutcome::Blocked &&
                        prediction.value().state == open;
    check(counter, passed,
          "WM6: custom Blocked model gives Blocked although rules say Moved");
}

void checkRecordedAction(TestCounter& counter) {
    const Belief belief{parsedOpen()};
    BudgetCounter budget(1);
    const WorldModel model;
    const std::optional<Prediction> prediction =
        model.predict(belief, Action::West, budget);
    const bool passed =
        prediction.has_value() && prediction.value().action == Action::West;
    check(counter, passed,
          "WM7: the prediction records the action asked for (West)");
}

void checkEmptyModel(TestCounter& counter) {
    const Belief belief{parsedOpen()};
    const ActionModel empty;
    const WorldModel fromEmpty(empty);
    const WorldModel standard;
    bool same = true;
    const Action actions[] = {Action::North, Action::East, Action::South,
                              Action::West};
    for (const Action action : actions) {
        BudgetCounter emptyBudget(1);
        BudgetCounter standardBudget(1);
        const std::optional<Prediction> fromEmptyPrediction =
            fromEmpty.predict(belief, action, emptyBudget);
        const std::optional<Prediction> standardPrediction =
            standard.predict(belief, action, standardBudget);
        if (!fromEmptyPrediction.has_value()) {
            same = false;
        }
        if (!standardPrediction.has_value()) {
            same = false;
        }
        if (fromEmptyPrediction.has_value() && standardPrediction.has_value()) {
            if (fromEmptyPrediction.value().outcome !=
                standardPrediction.value().outcome) {
                same = false;
            }
            if (fromEmptyPrediction.value().state !=
                standardPrediction.value().state) {
                same = false;
            }
        }
    }
    check(counter, same,
          "WM8: empty ActionModel gives the default predictions, all actions");
}

}  // namespace

void runWorldModelTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- World model ---\n";
    }
    checkDefaultPredictions(counter);
    checkBudgetUse(counter);
    checkBeliefUnchanged(counter);
    checkCustomModel(counter);
    checkRecordedAction(counter);
    checkEmptyModel(counter);
}

}  // namespace aih
