// Purpose: Runs and prints the fixed scripted demonstration
// of the grid world (plan Section 3.8).
// Introduced in: Stage 1c (rebuild)
// Architecture: The demo parses its hand-written layout,
// steps an Environment through hand-written actions and
// prints the worlds and outcomes as text. The actions are
// hand-written; nothing chooses them. There is no reasoning
// here of any kind.

#include "demo/scripted_demo.h"

#include <array>
#include <ostream>
#include <string>
#include <vector>

#include "world/action.h"
#include "world/environment.h"
#include "world/rules.h"
#include "world/world_state.h"
#include "world/world_text.h"

namespace aih {
namespace {

// Writes the ten text lines of one world to out.
void printWorld(std::ostream& out, const WorldState& state) {
    const std::vector<std::string> lines = renderWorld(state);
    for (const std::string& line : lines) {
        out << line << '\n';
    }
}

// Applies each action in order and writes one step line per
// action, starting with the blank line that separates the
// steps from the initial world above them.
void printSteps(std::ostream& out, Environment& environment,
                const std::array<Action, 5>& actions) {
    out << '\n';
    int stepNumber = 1;
    for (const Action action : actions) {
        const ActionResult result = environment.step(action);
        out << "Step " << stepNumber << ": " << actionName(action) << " -> "
            << outcomeName(result.outcome) << '\n';
        stepNumber = stepNumber + 1;
    }
}

}  // namespace

std::vector<std::string> demoLayout() {
    return {"##########", "#........#", "#........#", "#..A.O..D#",
            "#........#", "#........#", "#...##...#", "#........#",
            "#........#", "##########"};
}

void runScriptedDemo(std::ostream& out) {
    const ParseResult parsed = parseWorld(demoLayout());
    if (!parsed.world.has_value()) {
        out << "Demo layout is invalid: " << parsed.error << '\n';
        return;
    }
    Environment environment(parsed.world.value());
    // The five East actions are hand-written here in full;
    // nothing chooses them and no agent is involved.
    const std::array<Action, 5> actions = {
        Action::East, Action::East, Action::East, Action::East, Action::East};
    // The first thing written is an empty line, as in plan
    // Section 3.8, so the demo starts with a blank line.
    out << "\nInitial world:\n";
    printWorld(out, environment.state());
    printSteps(out, environment, actions);
    out << "\nFinal world:\n";
    printWorld(out, environment.state());
    if (isGoalReached(environment.state())) {
        out << "Goal reached: yes\n";
    } else {
        out << "Goal reached: no\n";
    }
    out << "Steps taken: " << environment.stepCount() << '\n';
}

}  // namespace aih
