// Purpose: Tests for parseWorld and isWall on parsed worlds
// (T1 to T20, plan Section 6.2).
// Introduced in: Stage 1a (rebuild)
// Architecture: Each check uses the layouts of test_layouts
// and, where the plan says so, withLine to change one line.
// Expected messages are the exact strings of ref_world.py and
// checks.py; a check description always starts with its id.

#include <string>
#include <vector>

#include "test_layouts.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/position.h"
#include "world/world_state.h"
#include "world/world_text.h"

namespace aih {
namespace {

// Parses a layout that these tests expect to succeed and
// returns its world, or an empty state when it does not.
WorldState parsed(const std::vector<std::string>& layout) {
    const ParseResult result = parseWorld(layout);
    if (result.world.has_value()) {
        return result.world.value();
    }
    return WorldState{};
}

// Returns only the error text of parsing the given lines.
std::string errorOf(const std::vector<std::string>& lines) {
    return parseWorld(lines).error;
}

void checkDemoBasics(TestCounter& counter) {
    const ParseResult result = parseWorld(demoLayout());
    check(counter, result.world.has_value() && result.error.empty(),
          "T1: DEMO parses with a world present and an empty error");
    if (result.world.has_value()) {
        const WorldState world = result.world.value();
        check(counter,
              world.agent == Position{3, 3} && world.object == Position{5, 3} &&
                  world.destination == Position{8, 3},
              "T2: DEMO agent (3,3), object (5,3), destination (8,3)");
        check(counter,
              isWall(world, Position{0, 0}) && isWall(world, Position{4, 6}) &&
                  isWall(world, Position{5, 6}) &&
                  !isWall(world, Position{1, 1}) &&
                  !isWall(world, Position{4, 5}),
              "T3: DEMO walls at (0,0), (4,6), (5,6); floor elsewhere");
    } else {
        check(counter, false,
              "T2: DEMO agent (3,3), object (5,3), destination (8,3)");
        check(counter, false,
              "T3: DEMO walls at (0,0), (4,6), (5,6); floor elsewhere");
    }
}

void checkLineCountAndLength(TestCounter& counter) {
    std::vector<std::string> nine = demoLayout();
    nine.pop_back();
    check(counter, errorOf(nine) == "A world must contain exactly 10 lines.",
          "T4: DEMO first 9 lines give the lines message");
    std::vector<std::string> eleven = demoLayout();
    eleven.push_back("##########");
    check(counter, errorOf(eleven) == "A world must contain exactly 10 lines.",
          "T5: DEMO plus an 11th line gives the lines message");
    check(counter,
          errorOf(withLine(demoLayout(), 4, "#.......#")) ==
              "Every world line must contain exactly 10 characters.",
          "T6: line 4 with 9 characters gives the length message");
    check(counter,
          errorOf(withLine(demoLayout(), 4, "#.........#")) ==
              "Every world line must contain exactly 10 characters.",
          "T7: line 4 with 11 characters gives the length message");
}

void checkCellAndCountErrors(TestCounter& counter) {
    check(counter,
          errorOf(withLine(demoLayout(), 9, "####.#####")) ==
              "Every border cell must be a wall (#).",
          "T8: line 9 with a floor border cell gives the border message");
    check(counter,
          errorOf(withLine(demoLayout(), 3, "#....O..D#")) ==
              "A world must contain exactly one agent (A or +).",
          "T9: DEMO with no agent gives the agent message");
    check(counter,
          errorOf(withLine(demoLayout(), 1, "#A.......#")) ==
              "A world must contain exactly one agent (A or +).",
          "T10: DEMO with two agents gives the agent message");
    check(counter,
          errorOf(withLine(demoLayout(), 3, "#..A....D#")) ==
              "A world must contain exactly one object (O or *).",
          "T11: DEMO with no object gives the object message");
    check(counter,
          errorOf(withLine(demoLayout(), 1, "#O.......#")) ==
              "A world must contain exactly one object (O or *).",
          "T12: DEMO with two objects gives the object message");
    check(counter,
          errorOf(withLine(demoLayout(), 3, "#..A.O...#")) ==
              "A world must contain exactly one destination (D, * or +).",
          "T13: DEMO with no destination gives the destination message");
    check(counter,
          errorOf(withLine(demoLayout(), 1, "#D.......#")) ==
              "A world must contain exactly one destination (D, * or +).",
          "T14: DEMO with two destinations gives the destination message");
    check(counter,
          errorOf(withLine(demoLayout(), 2, "#...x....#")) ==
              "The world contains an unknown character.",
          "T15: DEMO with x gives the unknown character message");
}

void checkStarAndPlus(TestCounter& counter) {
    const WorldState star = parsed(starLayout());
    check(counter,
          star.object == Position{8, 3} && star.destination == Position{8, 3} &&
              star.agent == Position{3, 3},
          "T16: STAR object and destination (8,3), agent (3,3)");
    const WorldState plus = parsed(plusLayout());
    check(counter,
          plus.agent == Position{8, 3} && plus.destination == Position{8, 3} &&
              plus.object == Position{3, 3},
          "T17: PLUS agent and destination (8,3), object (3,3)");
}

void checkEmptyAndLongInputs(TestCounter& counter) {
    const std::vector<std::string> empty;
    check(counter, errorOf(empty) == "A world must contain exactly 10 lines.",
          "T18: empty list of lines gives the lines message");
    const std::vector<std::string> blanks(10, "");
    check(counter,
          errorOf(blanks) ==
              "Every world line must contain exactly 10 characters.",
          "T19: ten empty strings give the length message");
    const std::vector<std::string> longLines(10, std::string(1000, '#'));
    check(counter,
          errorOf(longLines) ==
              "Every world line must contain exactly 10 characters.",
          "T20: ten lines of 1000 characters give the length message");
}

}  // namespace

void runWorldParseTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- World parse ---\n";
    }
    checkDemoBasics(counter);
    checkLineCountAndLength(counter);
    checkCellAndCountErrors(counter);
    checkStarAndPlus(counter);
    checkEmptyAndLongInputs(counter);
}

}  // namespace aih
