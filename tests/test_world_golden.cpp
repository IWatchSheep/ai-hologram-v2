// Purpose: Golden tests for the parser (G2, plan Section
// 6.7) and the malformed-file tests GN1 to GN6. G2: the
// rebuilt parser must reproduce all 1,009 cases of the
// golden file produced from the original C++.
// Introduced in: Stage 1a (rebuild); GN1 to GN6 in Stage 1b
// Architecture: The golden file is read with readLines from
// golden_files through AIH_SOURCE_DIR (decision D-063) and
// checked by the shared bounds-safe reader in
// golden_parse_reader.cpp, which GN1 to GN6 also use.

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

// One GN check: the reader's whole result must match the
// expected (ok, cases, problem) of plan Section 6.7 exactly.
void expectParseCheck(TestCounter& counter, const std::string& description,
                      const std::vector<std::string>& lines, bool ok, int cases,
                      const std::string& problem) {
    const GoldenCheck actual = checkParseGoldenLines(lines);
    const bool matched =
        actual.ok == ok && actual.cases == cases && actual.problem == problem;
    check(counter, matched, description);
}

// Returns header followed by the first count DEMO lines.
std::vector<std::string> withDemoLines(const std::string& header, int count) {
    std::vector<std::string> lines;
    lines.push_back(header);
    const std::vector<std::string> demo = demoLayout();
    for (int i = 0; i < count; i = i + 1) {
        lines.push_back(demo[static_cast<std::size_t>(i)]);
    }
    return lines;
}

void checkMalformedParse(TestCounter& counter) {
    const std::string emptyError =
        "expect ERROR A world must contain exactly 10 lines.";
    expectParseCheck(counter, "GN1: header asks for 10 lines but only 3 follow",
                     withDemoLines("case 1 lines 10", 3), false, 0,
                     "case 1: missing input lines");
    expectParseCheck(counter, "GN2: no expect line after the inputs",
                     {"case 1 lines 2", "a", "b"}, false, 0,
                     "case 1: missing expect line");
    expectParseCheck(counter, "GN3: expect line lacks the expect prefix",
                     {"case 1 lines 0", "result OK"}, false, 0,
                     "case 1: malformed expect line");
    expectParseCheck(counter, "GN4: negative line count is an invalid header",
                     {"case 1 lines -1", emptyError}, false, 0,
                     "line 1: invalid case header");
    expectParseCheck(counter, "GN5: non-integer line count is invalid",
                     {"case 1 lines x", emptyError}, false, 0,
                     "line 1: invalid case header");
    expectParseCheck(counter, "GN6: a correct empty case passes",
                     {"case 1 lines 0", emptyError}, true, 1, "");
}

void checkGoldenParseFile(TestCounter& counter) {
    const std::string path =
        std::string(AIH_SOURCE_DIR) +
        "/tools/reference/stage1/golden/world_parse_golden.txt";
    const std::vector<std::string> lines = readLines(path);
    if (lines.empty()) {
        check(counter, false,
              "G2: golden parse file missing or unreadable: " + path);
        return;
    }
    const GoldenCheck result = checkParseGoldenLines(lines);
    if (!result.ok) {
        check(counter, false, "G2: " + result.problem);
        return;
    }
    check(counter, result.cases == 1009,
          "G2: all golden parse cases match and there are exactly 1009 "
          "cases, found " +
              std::to_string(result.cases));
}

}  // namespace

void runWorldGoldenTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- World golden ---\n";
    }
    checkGoldenParseFile(counter);
    checkMalformedParse(counter);
}

}  // namespace aih
