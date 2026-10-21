// Purpose: Golden tests for prediction and comparison
// (G3, plan Section 6.5) and the malformed-file tests
// PN1 to PN7. G3: the rebuilt code must reproduce all
// 1,000 cases of the golden file from the original C++.
// Introduced in: Stage 2b (rebuild)
// Architecture: The golden file is read with readLines
// from golden_files through AIH_SOURCE_DIR (D-063) and
// checked by the shared bounds-safe reader in
// golden_prediction_reader.cpp, which PN1 to PN7 also use.
// Expected values come from checks.py (READER_CASES).

#include <cstddef>
#include <string>
#include <vector>

#include "golden_files.h"
#include "golden_readers.h"
#include "test_layouts.h"
#include "test_suites.h"
#include "test_support.h"

#ifndef AIH_SOURCE_DIR
#define AIH_SOURCE_DIR ""
#endif

namespace aih {
namespace {

// One PN check: the reader's whole result must match the
// expected (ok, cases, problem) of plan Section 6.5 exactly.
void expectPredictionCheck(TestCounter& counter, const std::string& description,
                           const std::vector<std::string>& lines, bool ok,
                           int cases, const std::string& problem) {
    const GoldenCheck actual = checkPredictionGoldenLines(lines);
    const bool matched =
        actual.ok == ok && actual.cases == cases && actual.problem == problem;
    check(counter, matched, description);
}

// Adds every row of source to the end of lines.
void appendLines(std::vector<std::string>& lines,
                 const std::vector<std::string>& source) {
    for (const std::string& row : source) {
        lines.push_back(row);
    }
}

// Builds header, belief rows, truth rows and a result line.
std::vector<std::string> caseLines(const std::string& header,
                                   const std::vector<std::string>& belief,
                                   const std::vector<std::string>& truth,
                                   const std::string& resultLine) {
    std::vector<std::string> lines;
    lines.push_back(header);
    appendLines(lines, belief);
    appendLines(lines, truth);
    if (!resultLine.empty()) {
        lines.push_back(resultLine);
    }
    return lines;
}

void checkMalformedPrediction(TestCounter& counter) {
    const std::string header = "case 1 action East differences 0";
    const std::string result = "result Moved Moved match";
    const std::vector<std::string> demo = demoLayout();
    expectPredictionCheck(
        counter, "PN1: unknown action name is an invalid case header",
        caseLines("case 1 action Up differences 0", demo, demo, result), false,
        0, "line 1: invalid case header");
    std::vector<std::string> fewLines;
    fewLines.push_back(header);
    for (int i = 0; i < 5; i = i + 1) {
        fewLines.push_back(demo[static_cast<std::size_t>(i)]);
    }
    expectPredictionCheck(counter, "PN2: only 5 of the 20 world lines follow",
                          fewLines, false, 0, "case 1: missing world lines");
    const std::vector<std::string> badBelief = withLine(demo, 0, "####.#####");
    expectPredictionCheck(
        counter, "PN3: belief with a floor border cell does not parse",
        caseLines(header, badBelief, demo, result), false, 0,
        "case 1: belief does not parse: Every border cell must be a wall (#).");
    expectPredictionCheck(counter, "PN4: no result line after the worlds",
                          caseLines(header, demo, demo, ""), false, 0,
                          "case 1: missing result line");
    expectPredictionCheck(
        counter, "PN5: result line lacks the result prefix",
        caseLines(header, demo, demo, "outcome Moved Moved match"), false, 0,
        "case 1: malformed result line");
    expectPredictionCheck(
        counter, "PN6: one difference line promised but none follows",
        caseLines("case 1 action East differences 1", demo, demo, result),
        false, 0, "case 1: missing difference lines");
    expectPredictionCheck(counter, "PN7: a correct case passes",
                          caseLines(header, demo, demo, result), true, 1, "");
}

void checkGoldenPredictionFile(TestCounter& counter) {
    const std::string path =
        std::string(AIH_SOURCE_DIR) +
        "/tools/reference/stage2/golden/prediction_golden.txt";
    const std::vector<std::string> lines = readLines(path);
    if (lines.empty()) {
        check(counter, false,
              "G3: golden prediction file missing or unreadable: " + path);
        return;
    }
    const GoldenCheck result = checkPredictionGoldenLines(lines);
    if (!result.ok) {
        check(counter, false, "G3: " + result.problem);
        return;
    }
    check(counter, result.cases == 1000,
          "G3: all golden prediction cases match and there are exactly "
          "1000 cases, found " +
              std::to_string(result.cases));
}

}  // namespace

void runPredictionGoldenTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Prediction golden ---\n";
    }
    checkGoldenPredictionFile(counter);
    checkMalformedPrediction(counter);
}

}  // namespace aih
