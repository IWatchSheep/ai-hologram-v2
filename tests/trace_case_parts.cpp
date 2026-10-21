// Purpose: Implements the line and word readers that
// readTraceCases uses, following ref_trace_reader.py.
// Introduced in: Stage 3g (rebuild)
// Architecture: Each helper checks the word count before
// reading any word and returns false on the first problem,
// so the caller in trace_cases.cpp can name the stage that
// failed with the reference's exact problem text.

#include "trace_case_parts.h"

#include <optional>
#include <string>
#include <vector>

#include "golden_files.h"
#include "trace_cases.h"
#include "trace_models.h"
#include "world/world_text.h"

namespace aih {

std::optional<int> traceInteger(const std::string& text) {
    int value = 0;
    if (!parseInteger(text, value) || value < 0) {
        return std::nullopt;
    }
    return value;
}

bool traceSettingsFromWords(const std::vector<std::string>& words,
                            AgentSettings& settings) {
    const std::optional<int> budget =
        words.size() == 5 ? traceInteger(words[1]) : std::nullopt;
    const std::optional<int> stops =
        words.size() == 5 ? traceInteger(words[2]) : std::nullopt;
    const std::optional<int> steps =
        words.size() == 5 ? traceInteger(words[3]) : std::nullopt;
    const std::optional<int> delay =
        words.size() == 5 ? traceInteger(words[4]) : std::nullopt;
    const bool ok = words.size() == 5 && words[0] == "settings" && budget &&
                    stops && steps && delay;
    if (ok) {
        settings = AgentSettings{budget.value(), stops.value(), steps.value(),
                                 delay.value()};
    }
    return ok;
}

bool traceWorldRows(const std::vector<std::string>& lines, int firstRow,
                    int caseNumber, const std::string& what, WorldState& world,
                    std::string& problem) {
    const std::string prefix = "case " + std::to_string(caseNumber) + ": ";
    if (firstRow + 10 > static_cast<int>(lines.size())) {
        problem = prefix + "missing " + what + " lines";
        return false;
    }
    const std::vector<std::string> rows(lines.begin() + firstRow,
                                        lines.begin() + firstRow + 10);
    const ParseResult parsed = parseWorld(rows);
    if (!parsed.world.has_value()) {
        problem = prefix + what + " does not parse: " + parsed.error;
        return false;
    }
    world = parsed.world.value();
    return true;
}

bool traceModelWordsOk(const std::vector<std::string>& words) {
    if (words.empty() || words[0] != "model") {
        return false;
    }
    const std::vector<std::string> spec(words.begin() + 1, words.end());
    return traceModelSpecOk(spec);
}

bool traceNotDeadFromWords(const std::vector<std::string>& words,
                           std::vector<Position>& cells) {
    const std::optional<int> count =
        words.size() > 1 ? traceInteger(words[1]) : std::nullopt;
    bool ok =
        words.size() >= 2 && words[0] == "notdead" && count &&
        words.size() == 2 + 2 * static_cast<std::size_t>(count.value_or(0));
    for (int k = 0; ok && k < count.value(); k = k + 1) {
        const std::size_t at = static_cast<std::size_t>(k);
        const std::optional<int> x = traceInteger(words[2 + 2 * at]);
        const std::optional<int> y = traceInteger(words[3 + 2 * at]);
        if (x && y) {
            cells.push_back(Position{x.value(), y.value()});
        } else {
            ok = false;
        }
    }
    return ok;
}

bool applyTraceSeed(TraceCase& traceCase,
                    const std::vector<std::string>& words) {
    Action action;
    const std::optional<int> x =
        words.size() > 2 ? traceInteger(words[2]) : std::nullopt;
    const std::optional<int> y =
        words.size() > 3 ? traceInteger(words[3]) : std::nullopt;
    const bool wall = words.size() == 6 && words[1] == "wall" && x && y;
    const bool states = words.size() == 6 &&
                        (words[4] == "blocked" || words[4] == "free") &&
                        (words[5] == "blocked" || words[5] == "free");
    if (wall && states) {
        traceCase.walls.push_back(TraceSeedWall{Position{x.value(), y.value()},
                                                words[4] == "blocked",
                                                words[5] == "blocked"});
        return true;
    }
    const std::optional<int> count =
        words.size() == 4 ? traceInteger(words[3]) : std::nullopt;
    if (words.size() == 4 && words[1] == "v1" && !traceCase.v1.has_value() &&
        actionFromName(words[2], action) && count) {
        traceCase.v1 = TraceSeedV1{action, count.value()};
        return true;
    }
    if (words.size() == 3 && actionFromName(words[2], action)) {
        if (words[1] == "committed" && !traceCase.committed.has_value()) {
            traceCase.committed = action;
            return true;
        }
        if (words[1] == "plan" && !traceCase.plan.has_value()) {
            traceCase.plan = action;
            return true;
        }
    }
    return false;
}

}  // namespace aih
