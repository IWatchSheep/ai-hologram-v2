// Purpose: Tests for the equivalence gate, part 2 (G8,
// G9, G10; plan 6.6): the 563 golden fault runs replayed.
// Introduced in: Stage 4g (rebuild)
// Architecture: The golden files were produced by the
// original C++ (orig_trace) and are never edited; G8
// replays every case through the rebuilt agents and
// compares every traced line, which is the second half
// of the M1.4 proof that the rebuild does exactly what
// the original did. G9 and G10 prove the exact case count
// is enforced, so a truncated gate cannot pass by matching.

#include <string>
#include <vector>

#include "fault_gate.h"
#include "golden_files.h"
#include "stage4_suites.h"
#include "test_support.h"

namespace aih {
namespace {
using Lines = std::vector<std::string>;

// The lines up to and including the nth "end" line: the
// first n cases, or blocks, of a golden file.
Lines firstBlocks(const Lines& lines, int count) {
    Lines result;
    int ends = 0;
    for (const std::string& line : lines) {
        result.push_back(line);
        if (line == "end") {
            ends = ends + 1;
        }
        if (ends == count) {
            break;
        }
    }
    return result;
}

// Reads the two golden fault files of Stage 4.
void readGoldenFaultFiles(Lines& caseLines, Lines& goldenLines) {
    const std::string folder =
        std::string(AIH_SOURCE_DIR) + "/tools/reference/stage4_rebuild/golden/";
    caseLines = readLines(folder + "fault_cases.txt");
    goldenLines = readLines(folder + "fault_traces.txt");
}

// G8: exactly 563 cases, and every block identical. On a
// failure the description shows the case, step and line
// with the expected and actual text, as G4 and G5 do.
void checkGate563(TestCounter& counter) {
    Lines caseLines;
    Lines goldenLines;
    readGoldenFaultFiles(caseLines, goldenLines);
    const std::string failure =
        runFaultGate(caseLines, goldenLines, kGoldenFaultCaseCount);
    std::string description = "G8: all 563 golden fault blocks are identical";
    if (!failure.empty()) {
        description += " (" + failure + ")";
    }
    check(counter, failure.empty(), description);
}

// G9 and G10: the first three matching cases fail only
// because 563 are required, and pass when 3 are required.
void checkTruncatedGate(TestCounter& counter) {
    Lines caseLines;
    Lines goldenLines;
    readGoldenFaultFiles(caseLines, goldenLines);
    const Lines threeCases = firstBlocks(caseLines, 3);
    const Lines threeTraces = firstBlocks(goldenLines, 3);
    const std::string failure =
        runFaultGate(threeCases, threeTraces, kGoldenFaultCaseCount);
    check(counter, failure == "case file has 3 cases, required 563",
          "G9: three matching cases fail with the required count");
    const std::string control = runFaultGate(threeCases, threeTraces, 3);
    check(counter, control.empty(),
          "G10: the same three cases pass when 3 are required");
}
}  // namespace

void runFaultGateTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Fault gate ---\n";
    }
    checkGate563(counter);
    checkTruncatedGate(counter);
}

}  // namespace aih
