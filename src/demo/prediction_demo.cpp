// Purpose: Runs and prints the two prediction demonstrations
// of plan Section 3.7: one correct belief and one wrong one.
// Introduced in: Stage 2c (rebuild)
// Architecture: Each demo parses the shared demo layout,
// predicts each hand-written action from its belief through
// the budgeted WorldModel, steps its own Environment,
// compares prediction with observation and prints the step
// and difference lines from the shared comparison code.

#include "demo/prediction_demo.h"

#include <array>
#include <optional>
#include <ostream>
#include <string>
#include <vector>

#include "compare/difference_text.h"
#include "compare/state_difference.h"
#include "demo/scripted_demo.h"
#include "model/belief.h"
#include "model/budget_counter.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/environment.h"
#include "world/rules.h"
#include "world/world_state.h"
#include "world/world_text.h"

namespace aih {
namespace {

// Writes every difference line of one comparison to out.
void printDifferenceLines(std::ostream& out,
                          const PredictionComparison& comparison) {
    const std::vector<std::string> lines =
        describeDifferences(comparison.state);
    for (const std::string& line : lines) {
        out << line << '\n';
    }
}

// Predicts, steps and compares each action in order, and
// writes one step line per action. Returns how many steps
// did not match. Stops early when no prediction is available.
int runPredictionSteps(std::ostream& out, Environment& environment,
                       Belief& belief, const WorldModel& model,
                       BudgetCounter& budget) {
    // The actions are hand-written here in full; nothing
    // chooses them and no agent is involved.
    const std::array<Action, 5> actions = {
        Action::East, Action::East, Action::East, Action::East, Action::East};
    int mismatchCount = 0;
    int stepNumber = 1;
    for (const Action action : actions) {
        const std::optional<Prediction> prediction =
            model.predict(belief, action, budget);
        if (!prediction.has_value()) {
            out << "No prediction available; stopping this demo.\n";
            break;
        }
        const ActionResult observed = environment.step(action);
        const PredictionComparison comparison =
            comparePrediction(prediction.value(), observed);
        out << "Step " << stepNumber << ": " << actionName(action)
            << " | predicted " << outcomeName(comparison.predictedOutcome)
            << ", observed " << outcomeName(comparison.observedOutcome)
            << " | ";
        if (matches(comparison)) {
            out << "match\n";
        } else {
            out << "MISMATCH\n";
            mismatchCount = mismatchCount + 1;
        }
        printDifferenceLines(out, comparison);
        // Replacing the belief with the observed state is a
        // demonstration shortcut, not a correction policy.
        belief.state = observed.state;
        stepNumber = stepNumber + 1;
    }
    return mismatchCount;
}

// Runs one prediction demo under its title, with its own
// Environment and BudgetCounter(10). When addWrongWall is
// true, the belief wrongly has a wall at (6, 3).
void runOnePredictionDemo(std::ostream& out, const std::string& title,
                          bool addWrongWall) {
    const ParseResult parsed = parseWorld(demoLayout());
    if (!parsed.world.has_value()) {
        out << "Prediction demo layout is invalid: " << parsed.error << '\n';
        return;
    }
    Environment environment(parsed.world.value());
    Belief belief{parsed.world.value()};
    if (addWrongWall) {
        // The wrong belief is hand-written here; nothing
        // chooses it. This is not fault injection.
        belief.state.walls[3][6] = true;
    }
    const WorldModel model;
    BudgetCounter budget(10);
    out << '\n' << title << '\n';
    const int mismatchCount =
        runPredictionSteps(out, environment, belief, model, budget);
    out << "Mismatches: " << mismatchCount << '\n';
    out << "Model evaluations used: " << budget.used() << '\n';
}

}  // namespace

void runPredictionDemos(std::ostream& out) {
    runOnePredictionDemo(out, "Prediction demo 1: correct belief", false);
    runOnePredictionDemo(
        out, "Prediction demo 2: belief wrongly includes a wall at (6, 3)",
        true);
    out << "Note: in these demos the belief is replaced by the observation "
           "after every step.\n";
    out << "How beliefs are corrected is decided in Stage 3.\n";
}

}  // namespace aih
