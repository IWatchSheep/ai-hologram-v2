// Purpose: Golden tests for the movement rules (G1, plan
// Section 6.7) and the malformed-file tests GR1 to GR5.
// G1: the rebuilt rules must reproduce all 5,184 cases of
// the golden file produced from the original C++.
// Introduced in: Stage 1b (rebuild)
// Architecture: The golden file is read with readLines from
// golden_files through AIH_SOURCE_DIR (decision D-063) and
// checked by the shared bounds-safe reader in
// golden_rules_reader.cpp, which GR1 to GR5 also use.

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

// One GR check: the reader's whole result must match the
// expected (ok, cases, problem) of plan Section 6.7 exactly.
void expectRulesCheck(TestCounter& counter, const std::string& description,
                      const std::vector<std::string>& lines, bool ok, int cases,
                      const std::string& problem) {
    const GoldenCheck actual = checkRulesGoldenLines(lines);
    const bool matched =
        actual.ok == ok && actual.cases == cases && actual.problem == problem;
    check(counter, matched, description);
}

// Returns "layout L1" followed by count DEMO lines and,
// when given, one case line.
std::vector<std::string> layoutLines(int count, const std::string& caseLine) {
    std::vector<std::string> lines;
    lines.push_back("layout L1");
    const std::vector<std::string> demo = demoLayout();
    for (int i = 0; i < count; i = i + 1) {
        lines.push_back(demo[static_cast<std::size_t>(i)]);
    }
    if (!caseLine.empty()) {
        lines.push_back(caseLine);
    }
    return lines;
}

void checkMalformedRules(TestCounter& counter) {
    expectRulesCheck(counter, "GR1: layout with only 4 of its 10 lines",
                     layoutLines(4, ""), false, 0,
                     "line 1: truncated layout L1");
    expectRulesCheck(counter, "GR2: case line with no arrow is malformed",
                     layoutLines(10, "L1 3 3 5 3"), false, 0,
                     "line 12: malformed case line");
    expectRulesCheck(counter, "GR3: case naming an unknown layout L9",
                     layoutLines(10, "L9 3 3 5 3 East -> Moved 4 3 5 3 0"),
                     false, 0, "line 12: unknown layout L9");
    const std::string problem =
        "line 12: expected [L1 3 3 5 3 East -> Moved 4 3 5 3] actual "
        "[L1 3 3 5 3 East -> Moved 4 3 5 3 0]";
    expectRulesCheck(counter, "GR4: case line missing its goal value",
                     layoutLines(10, "L1 3 3 5 3 East -> Moved 4 3 5 3"), false,
                     1, problem);
    expectRulesCheck(counter, "GR5: a correct case line passes",
                     layoutLines(10, "L1 3 3 5 3 East -> Moved 4 3 5 3 0"),
                     true, 1, "");
}

void checkGoldenRulesFile(TestCounter& counter) {
    const std::string path =
        std::string(AIH_SOURCE_DIR) +
        "/tools/reference/stage1/golden/world_rules_golden.txt";
    const std::vector<std::string> lines = readLines(path);
    if (lines.empty()) {
        check(counter, false,
              "G1: golden rules file missing or unreadable: " + path);
        return;
    }
    const GoldenCheck result = checkRulesGoldenLines(lines);
    if (!result.ok) {
        check(counter, false, "G1: " + result.problem);
        return;
    }
    check(counter, result.cases == 5184,
          "G1: all golden rules cases match and there are exactly 5184 "
          "cases, found " +
              std::to_string(result.cases));
}

}  // namespace

void runWorldGoldenRulesTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- World golden rules ---\n";
    }
    checkGoldenRulesFile(counter);
    checkMalformedRules(counter);
}

}  // namespace aih
