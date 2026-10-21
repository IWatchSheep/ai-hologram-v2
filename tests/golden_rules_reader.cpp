// Purpose: Implements the bounds-safe reader for the
// golden rules file (plan Section 6.7).
// Introduced in: Stage 1b (rebuild)
// Architecture: G1 and the malformed-file tests GR1 to GR5
// share this reader. It follows ref_golden_readers.py
// exactly: the same order of checks and problem messages,
// checking that a line exists before reading it.

#include "golden_readers.h"

#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "golden_files.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"
#include "world/world_text.h"

namespace aih {
namespace {

// Splits text at every single space, keeping empty words,
// exactly like the reference's split(' ').
std::vector<std::string> splitWords(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
    for (const char character : text) {
        if (character == ' ') {
            words.push_back(word);
            word.clear();
        } else {
            word.push_back(character);
        }
    }
    words.push_back(word);
    return words;
}

// Returns whether line starts with the whole prefix.
bool startsWith(const std::string& line, const std::string& prefix) {
    if (line.size() < prefix.size()) {
        return false;
    }
    return line.compare(0, prefix.size(), prefix) == 0;
}

// Turns one of the four action names into its Action, by
// comparing with actionName, the one place names are given.
bool actionForName(const std::string& name, Action& action) {
    const Action actions[] = {Action::North, Action::East, Action::South,
                              Action::West};
    for (const Action candidate : actions) {
        if (actionName(candidate) == name) {
            action = candidate;
            return true;
        }
    }
    return false;
}

// Reads "<id> <ax> <ay> <ox> <oy> <Action>" from a left side.
bool readCaseLeft(const std::string& left, std::string& layoutId,
                  Position& agent, Position& object, Action& action) {
    const std::vector<std::string> parts = splitWords(left);
    if (parts.size() != 6) {
        return false;
    }
    if (!actionForName(parts[5], action)) {
        return false;
    }
    std::vector<int> numbers;
    for (std::size_t i = 1; i <= 4; i = i + 1) {
        int value = 0;
        if (!parseInteger(parts[i], value)) {
            return false;
        }
        numbers.push_back(value);
    }
    layoutId = parts[0];
    agent = Position{numbers[0], numbers[1]};
    object = Position{numbers[2], numbers[3]};
    return true;
}

// Rebuilds one case line by applying its action to a copy
// of the layout with the agent and object moved as stated.
std::string formatRuleCase(const WorldState& layout, const std::string& left,
                           Position agent, Position object, Action action) {
    WorldState world = layout;
    world.agent = agent;
    world.object = object;
    const ActionResult applied = applyAction(world, action);
    const int goal = isGoalReached(applied.state) ? 1 : 0;
    std::string text = left + " -> " + outcomeName(applied.outcome) + " ";
    text += std::to_string(applied.state.agent.x) + " ";
    text += std::to_string(applied.state.agent.y) + " ";
    text += std::to_string(applied.state.object.x) + " ";
    text += std::to_string(applied.state.object.y) + " ";
    text += std::to_string(goal);
    return text;
}

// Reads one layout line and its 10 lines, moving index
// past them. Returns false after setting result.problem.
bool readLayout(const std::vector<std::string>& lines, std::size_t& index,
                std::map<std::string, WorldState>& layouts,
                GoldenCheck& result) {
    const std::string layoutId = lines[index].substr(7);
    if (index + 1 + static_cast<std::size_t>(gridSize) > lines.size()) {
        result.problem = "line " + std::to_string(index + 1) +
                         ": truncated layout " + layoutId;
        return false;
    }
    const std::ptrdiff_t begin = static_cast<std::ptrdiff_t>(index + 1);
    const std::ptrdiff_t end = static_cast<std::ptrdiff_t>(
        index + 1 + static_cast<std::size_t>(gridSize));
    const std::vector<std::string> rows(lines.begin() + begin,
                                        lines.begin() + end);
    const ParseResult parsed = parseWorld(rows);
    if (!parsed.world.has_value()) {
        result.problem = "line " + std::to_string(index + 1) + ": layout " +
                         layoutId + " does not parse: " + parsed.error;
        return false;
    }
    layouts[layoutId] = parsed.world.value();
    index = index + 1 + static_cast<std::size_t>(gridSize);
    return true;
}

// Checks one case line and moves index past it on success.
// Returns false after setting result.problem.
bool checkRuleCase(const std::vector<std::string>& lines, std::size_t& index,
                   const std::map<std::string, WorldState>& layouts,
                   GoldenCheck& result) {
    const std::string line = lines[index];
    const std::size_t arrow = line.find(" -> ");
    if (arrow == std::string::npos) {
        result.problem =
            "line " + std::to_string(index + 1) + ": malformed case line";
        return false;
    }
    const std::string left = line.substr(0, arrow);
    std::string layoutId;
    Position agent{};
    Position object{};
    Action action = Action::North;
    if (!readCaseLeft(left, layoutId, agent, object, action)) {
        result.problem =
            "line " + std::to_string(index + 1) + ": malformed case line";
        return false;
    }
    const std::map<std::string, WorldState>::const_iterator found =
        layouts.find(layoutId);
    if (found == layouts.end()) {
        result.problem = "line " + std::to_string(index + 1) +
                         ": unknown layout " + layoutId;
        return false;
    }
    const std::string actual =
        formatRuleCase(found->second, left, agent, object, action);
    result.cases = result.cases + 1;
    if (actual != line) {
        result.problem = "line " + std::to_string(index + 1) + ": expected [" +
                         line + "] actual [" + actual + "]";
        return false;
    }
    index = index + 1;
    return true;
}

}  // namespace

GoldenCheck checkRulesGoldenLines(const std::vector<std::string>& lines) {
    GoldenCheck result;
    std::map<std::string, WorldState> layouts;
    std::size_t index = 0;
    while (index < lines.size()) {
        if (startsWith(lines[index], "layout ")) {
            if (!readLayout(lines, index, layouts, result)) {
                return result;
            }
        } else if (!checkRuleCase(lines, index, layouts, result)) {
            return result;
        }
    }
    result.ok = true;
    return result;
}

}  // namespace aih
