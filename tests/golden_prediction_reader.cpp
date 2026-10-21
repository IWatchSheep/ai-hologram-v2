// Purpose: Implements the bounds-safe reader for the
// golden prediction file (plan Section 6.5).
// Introduced in: Stage 2b (rebuild)
// Architecture: G3 and the malformed-file tests PN1 to PN7
// share this reader. It follows ref_prediction_reader.py
// exactly: the same order of checks and problem messages,
// checking that a line exists before reading it, and it
// never throws. Text helpers come from golden_files (R4).

#include "golden_readers.h"

#include <optional>
#include <string>
#include <vector>

#include "compare/difference_text.h"
#include "golden_files.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/rules.h"
#include "world/world_state.h"
#include "world/world_text.h"

namespace aih {
namespace {

// The "case <n>: " start of every problem in one case.
std::string casePrefix(int number) {
    return "case " + std::to_string(number) + ": ";
}

// Reads "case <n> action <Action> differences <k>".
bool readHeader(const std::string& line, int& number, Action& action,
                int& count) {
    const std::vector<std::string> parts = splitWords(line);
    if (parts.size() != 6) {
        return false;
    }
    if (parts[0] != "case" || parts[2] != "action" ||
        parts[4] != "differences") {
        return false;
    }
    if (!actionFromName(parts[3], action)) {
        return false;
    }
    if (!parseInteger(parts[1], number)) {
        return false;
    }
    if (!parseInteger(parts[5], count)) {
        return false;
    }
    return count >= 0;
}

// Reads the 20 world lines after a header, moving index.
bool readCaseWorlds(const std::vector<std::string>& lines, int& index,
                    int number, WorldState& belief, WorldState& truth,
                    GoldenCheck& result) {
    if (index + 2 * gridSize > static_cast<int>(lines.size())) {
        result.problem = casePrefix(number) + "missing world lines";
        return false;
    }
    WorldState* worlds[] = {&belief, &truth};
    const std::string labels[] = {"belief", "true world"};
    for (int which = 0; which < 2; which = which + 1) {
        const int begin = index + which * gridSize;
        const std::vector<std::string> rows(lines.begin() + begin,
                                            lines.begin() + begin + gridSize);
        const ParseResult parsed = parseWorld(rows);
        if (!parsed.world.has_value()) {
            result.problem = casePrefix(number) + labels[which] +
                             " does not parse: " + parsed.error;
            return false;
        }
        *worlds[which] = parsed.world.value();
    }
    index = index + 2 * gridSize;
    return true;
}

// Reads the result line and its difference lines.
bool readExpected(const std::vector<std::string>& lines, int& index, int number,
                  int count, std::string& resultLine,
                  std::vector<std::string>& expected, GoldenCheck& result) {
    if (index >= static_cast<int>(lines.size())) {
        result.problem = casePrefix(number) + "missing result line";
        return false;
    }
    if (!startsWith(lines[index], "result ")) {
        result.problem = casePrefix(number) + "malformed result line";
        return false;
    }
    resultLine = lines[index];
    index = index + 1;
    if (index + count > static_cast<int>(lines.size())) {
        result.problem = casePrefix(number) + "missing difference lines";
        return false;
    }
    expected.assign(lines.begin() + index, lines.begin() + index + count);
    index = index + count;
    return true;
}

// Computes the result line and difference lines of one case.
void actualLines(const WorldState& beliefWorld, const WorldState& truth,
                 Action action, std::string& resultLine,
                 std::vector<std::string>& differences) {
    const Belief belief{beliefWorld};
    BudgetCounter budget(1);
    const WorldModel model;
    const std::optional<Prediction> prediction =
        model.predict(belief, action, budget);
    if (!prediction.has_value()) {
        return;
    }
    const ActionResult observed = applyAction(truth, action);
    const PredictionComparison comparison =
        comparePrediction(prediction.value(), observed);
    const std::string verdict = matches(comparison) ? "match" : "MISMATCH";
    resultLine = "result " + outcomeName(comparison.predictedOutcome) + " " +
                 outcomeName(comparison.observedOutcome) + " " + verdict;
    differences = describeDifferences(comparison.state);
}

// Compares one case in the reference's order, returning
// the problem text, or an empty string when it matches.
std::string checkCaseLines(int number, int count, const std::string& resultLine,
                           const std::vector<std::string>& expected,
                           const std::string& actualResult,
                           const std::vector<std::string>& actual) {
    if (resultLine != actualResult) {
        return casePrefix(number) + "expected [" + resultLine + "] actual [" +
               actualResult + "]";
    }
    if (static_cast<int>(actual.size()) != count) {
        return casePrefix(number) + "expected " + std::to_string(count) +
               " difference lines, actual " + std::to_string(actual.size());
    }
    for (int j = 0; j < count; j = j + 1) {
        if (expected[j] != actual[j]) {
            return casePrefix(number) + "difference line " +
                   std::to_string(j + 1) + ": expected [" + expected[j] +
                   "] actual [" + actual[j] + "]";
        }
    }
    return "";
}

}  // namespace

GoldenCheck checkPredictionGoldenLines(const std::vector<std::string>& lines) {
    GoldenCheck result;
    int index = 0;
    while (index < static_cast<int>(lines.size())) {
        int number = 0;
        Action action = Action::North;
        int count = 0;
        if (!readHeader(lines[index], number, action, count)) {
            result.problem =
                "line " + std::to_string(index + 1) + ": invalid case header";
            return result;
        }
        index = index + 1;
        WorldState belief{};
        WorldState truth{};
        if (!readCaseWorlds(lines, index, number, belief, truth, result)) {
            return result;
        }
        std::string resultLine;
        std::vector<std::string> expected;
        if (!readExpected(lines, index, number, count, resultLine, expected,
                          result)) {
            return result;
        }
        result.cases = result.cases + 1;
        std::string actualResult;
        std::vector<std::string> actual;
        actualLines(belief, truth, action, actualResult, actual);
        const std::string problem = checkCaseLines(
            number, count, resultLine, expected, actualResult, actual);
        if (!problem.empty()) {
            result.problem = problem;
            return result;
        }
    }
    result.ok = true;
    return result;
}

}  // namespace aih
