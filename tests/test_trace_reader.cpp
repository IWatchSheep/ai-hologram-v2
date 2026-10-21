// Purpose: Tests for the gate's readers (TN1 to TN15, plan
// Section 6.7), with the exact cases of checks.py.
// Introduced in: Stage 3g (rebuild)
// Architecture: Each malformed case must give exactly the
// reference's (ok, count, problem), which is what makes the
// readers safe to run on a golden file that may be wrong:
// they report the first problem instead of reading past
// the end of the file.

#include <string>
#include <vector>

#include "demo/scripted_demo.h"
#include "test_suites.h"
#include "test_support.h"
#include "trace_blocks.h"
#include "trace_cases.h"

namespace aih {
namespace {
using Lines = std::vector<std::string>;

// The positive case CASE_D of checks.py: the demo layout.
Lines demoCaseLines() {
    Lines lines{"case demo1", "agent C1", "settings 200 5 100 3", "world"};
    const Lines layout = demoLayout();
    lines.insert(lines.end(), layout.begin(), layout.end());
    lines.push_back("belief true");
    lines.push_back("model standard");
    lines.push_back("notdead 0");
    lines.push_back("end");
    return lines;
}

// A copy of lines with one line replaced.
Lines replaced(Lines lines, std::size_t index, const std::string& text) {
    lines[index] = text;
    return lines;
}

// A copy holding only the first count lines.
Lines firstLines(const Lines& lines, std::size_t count) {
    return Lines(lines.begin(), lines.begin() + count);
}

// One reader check: the result must be exactly the
// expected (ok, case count, problem) of checks.py.
void checkRead(TestCounter& counter, const std::string& id, const Lines& lines,
               bool ok, int count, const std::string& problem) {
    const TraceCaseRead result = readTraceCases(lines);
    const int actualCount =
        result.ok ? static_cast<int>(result.cases.size()) : 0;
    const bool good =
        result.ok == ok && actualCount == count && result.problem == problem;
    std::string description = id + ": readTraceCases result";
    if (!good) {
        description += " (actual problem [" + result.problem + "])";
    }
    check(counter, good, description);
}

// One split check, with the same expected form.
void checkSplit(TestCounter& counter, const std::string& id, const Lines& lines,
                bool ok, int count, const std::string& problem) {
    const TraceBlockSplit result = splitTraceBlocks(lines);
    const int actualCount =
        result.ok ? static_cast<int>(result.blocks.size()) : 0;
    const bool good =
        result.ok == ok && actualCount == count && result.problem == problem;
    std::string description = id + ": splitTraceBlocks result";
    if (!good) {
        description += " (actual problem [" + result.problem + "])";
    }
    check(counter, good, description);
}

// The readTraceCases checks TN1 to TN12 on CASE_D.
void checkReaderCases(TestCounter& counter, const Lines& demo) {
    Lines twoCases = demo;
    twoCases.insert(twoCases.end(), demo.begin(), demo.end());
    Lines badHeader{"case"};
    badHeader.insert(badHeader.end(), demo.begin() + 1, demo.end());
    checkRead(counter, "TN1", badHeader, false, 0,
              "line 1: expected a case header");
    checkRead(counter, "TN2", firstLines(demo, 1), false, 0,
              "case 1: missing agent line");
    checkRead(counter, "TN3", replaced(demo, 1, "agent C2"), false, 0,
              "case 1: malformed agent line");
    checkRead(counter, "TN4", replaced(demo, 2, "settings 200 5 100"), false, 0,
              "case 1: malformed settings line");
    checkRead(counter, "TN5", firstLines(demo, 8), false, 0,
              "case 1: missing world lines");
    checkRead(counter, "TN6", replaced(demo, 4, "####.#####"), false, 0,
              "case 1: world does not parse: Every border cell must be "
              "a wall (#).");
    checkRead(counter, "TN7", replaced(demo, 14, "belief maybe"), false, 0,
              "case 1: malformed belief line");
    checkRead(counter, "TN8", replaced(demo, 15, "model swap East"), false, 0,
              "case 1: malformed model line");
    checkRead(counter, "TN9", replaced(demo, 16, "notdead 1 1"), false, 0,
              "case 1: malformed notdead line");
    Lines badSeed = firstLines(demo, 17);
    badSeed.push_back("seed wall 4 3 blocked open");
    badSeed.push_back("end");
    checkRead(counter, "TN10", badSeed, false, 0,
              "case 1: malformed seed line");
    checkRead(counter, "TN11", firstLines(demo, 17), false, 0,
              "case 1: missing end line");
    checkRead(counter, "TN12", twoCases, true, 2, "");
}

}  // namespace

void runTraceReaderTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Trace reader ---\n";
    }
    checkReaderCases(counter, demoCaseLines());
    checkSplit(counter, "TN13", Lines{"case demo1", "step 1"}, false, 0,
               "trace demo1: missing end line");
    checkSplit(counter, "TN14", Lines{"step 1"}, false, 0,
               "line 1: expected a case header");
    const std::string difference = firstTraceDifference(
        "x", Lines{"case x", "step 1", "decision East", "end"},
        Lines{"case x", "step 1", "decision West", "end"});
    check(counter,
          difference ==
              "case x, step 1, line 3: expected [decision East] "
              "actual [decision West]",
          "TN15: firstTraceDifference names case, step and line");
}

}  // namespace aih
