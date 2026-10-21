// Purpose: Golden tests for the parser (G2, plan Section
// 6.7): the rebuilt parser must reproduce all 1,009 cases of
// the golden file produced from the original C++.
// Introduced in: Stage 1a (rebuild)
// Architecture: The golden file is read with readLines from
// golden_files through AIH_SOURCE_DIR (decision D-063). Each
// case is parsed again here and formatted exactly like its
// expect line. G1 (rule golden values) arrives in Stage 1b.

#include <sstream>
#include <string>
#include <vector>

#include "golden_files.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/world_text.h"

#ifndef AIH_SOURCE_DIR
#define AIH_SOURCE_DIR ""
#endif

namespace aih {
namespace {

// What checking the whole golden file found.
struct GoldenOutcome {
    int cases = 0;
    bool matched = true;
    int mismatchCase = 0;
    std::string expected;
    std::string actual;
};

// Reads "case <n> lines <k>" into its two numbers.
bool parseHeader(const std::string& line, int& caseNumber, int& lineCount) {
    std::istringstream stream(line);
    std::string word;
    std::string linesWord;
    if (!(stream >> word)) {
        return false;
    }
    if (!(stream >> caseNumber)) {
        return false;
    }
    if (!(stream >> linesWord)) {
        return false;
    }
    if (!(stream >> lineCount)) {
        return false;
    }
    return word == "case" && linesWord == "lines";
}

// Formats one parse result exactly like an expect line:
// "OK <ax> <ay> <ox> <oy> <dx> <dy>" with the round-trip
// suffix when rendering differs, or "ERROR <message>".
std::string formatResult(const std::vector<std::string>& inputs) {
    const ParseResult result = parseWorld(inputs);
    if (!result.world.has_value()) {
        return "ERROR " + result.error;
    }
    const WorldState world = result.world.value();
    std::string text = "OK " + std::to_string(world.agent.x) + " ";
    text += std::to_string(world.agent.y) + " ";
    text += std::to_string(world.object.x) + " ";
    text += std::to_string(world.object.y) + " ";
    text += std::to_string(world.destination.x) + " ";
    text += std::to_string(world.destination.y);
    if (renderWorld(world) != inputs) {
        text += " ROUNDTRIP-DIFFERS";
    }
    return text;
}

// Walks every case in the golden lines and compares formats.
GoldenOutcome checkCases(const std::vector<std::string>& lines) {
    GoldenOutcome outcome;
    std::size_t index = 0;
    while (index < lines.size()) {
        int caseNumber = 0;
        int lineCount = 0;
        if (!parseHeader(lines[index], caseNumber, lineCount)) {
            outcome.matched = false;
            return outcome;
        }
        index = index + 1;
        std::vector<std::string> inputs;
        for (int i = 0; i < lineCount; i = i + 1) {
            inputs.push_back(lines[index]);
            index = index + 1;
        }
        outcome.expected = lines[index].substr(7);
        index = index + 1;
        outcome.actual = formatResult(inputs);
        outcome.cases = outcome.cases + 1;
        if (outcome.actual != outcome.expected) {
            outcome.matched = false;
            outcome.mismatchCase = caseNumber;
            return outcome;
        }
    }
    return outcome;
}

}  // namespace

void runWorldGoldenTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- World golden ---\n";
    }
    const std::string path =
        std::string(AIH_SOURCE_DIR) +
        "/tools/reference/stage1/golden/world_parse_golden.txt";
    const std::vector<std::string> lines = readLines(path);
    if (lines.empty()) {
        check(counter, false,
              "G2: golden parse file missing or unreadable: " + path);
        return;
    }
    const GoldenOutcome outcome = checkCases(lines);
    if (!outcome.matched && outcome.mismatchCase != 0) {
        check(counter, false,
              "G2: case " + std::to_string(outcome.mismatchCase) +
                  " expected [" + outcome.expected + "] actual [" +
                  outcome.actual + "]");
        return;
    }
    check(counter, outcome.matched && outcome.cases == 1009,
          "G2: all golden parse cases match and there are exactly 1009 "
          "cases, found " +
              std::to_string(outcome.cases));
}

}  // namespace aih
