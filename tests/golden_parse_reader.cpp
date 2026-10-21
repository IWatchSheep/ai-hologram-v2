// Purpose: Implements the bounds-safe reader for the
// golden parser file (plan Section 6.7).
// Introduced in: Stage 1b (rebuild)
// Architecture: This reader moved out of
// test_world_golden.cpp so G2 and the malformed-file tests
// GN1 to GN6 share it. It follows ref_golden_readers.py
// exactly: the same order of checks and problem messages.

#include "golden_readers.h"

#include <cstddef>
#include <string>
#include <vector>

#include "golden_files.h"
#include "world/world_state.h"
#include "world/world_text.h"

namespace aih {
namespace {

// Reads "case <n> lines <k>" with integers and k >= 0.
bool readCaseHeader(const std::string& line, int& number, int& count) {
    const std::vector<std::string> parts = splitWords(line);
    if (parts.size() != 4) {
        return false;
    }
    if (parts[0] != "case" || parts[2] != "lines") {
        return false;
    }
    if (!parseInteger(parts[1], number)) {
        return false;
    }
    if (!parseInteger(parts[3], count)) {
        return false;
    }
    return count >= 0;
}

// Formats one parse result exactly like an expect line:
// "OK <ax> <ay> <ox> <oy> <dx> <dy>" with the round-trip
// suffix when rendering differs, or "ERROR <message>".
std::string formatParseResult(const std::vector<std::string>& inputs) {
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

// Reads one case header and its input lines, moving index
// past them. Returns false after setting result.problem.
bool readCaseInputs(const std::vector<std::string>& lines, std::size_t& index,
                    int& number, std::vector<std::string>& rows,
                    GoldenCheck& result) {
    int count = 0;
    if (!readCaseHeader(lines[index], number, count)) {
        result.problem =
            "line " + std::to_string(index + 1) + ": invalid case header";
        return false;
    }
    index = index + 1;
    if (index + static_cast<std::size_t>(count) > lines.size()) {
        result.problem =
            "case " + std::to_string(number) + ": missing input lines";
        return false;
    }
    const std::ptrdiff_t begin = static_cast<std::ptrdiff_t>(index);
    const std::ptrdiff_t end =
        static_cast<std::ptrdiff_t>(index + static_cast<std::size_t>(count));
    rows.assign(lines.begin() + begin, lines.begin() + end);
    index = index + static_cast<std::size_t>(count);
    return true;
}

// Checks the expect line after one case's inputs and moves
// index past it. Returns false after setting result.problem.
bool checkCaseExpect(const std::vector<std::string>& lines, std::size_t& index,
                     int number, const std::vector<std::string>& rows,
                     GoldenCheck& result) {
    if (index >= lines.size()) {
        result.problem =
            "case " + std::to_string(number) + ": missing expect line";
        return false;
    }
    if (!startsWith(lines[index], "expect ")) {
        result.problem =
            "case " + std::to_string(number) + ": malformed expect line";
        return false;
    }
    const std::string expected = lines[index].substr(7);
    index = index + 1;
    const std::string actual = formatParseResult(rows);
    result.cases = result.cases + 1;
    if (actual != expected) {
        result.problem = "case " + std::to_string(number) + ": expected [" +
                         expected + "] actual [" + actual + "]";
        return false;
    }
    return true;
}

}  // namespace

GoldenCheck checkParseGoldenLines(const std::vector<std::string>& lines) {
    GoldenCheck result;
    std::size_t index = 0;
    while (index < lines.size()) {
        int number = 0;
        std::vector<std::string> rows;
        if (!readCaseInputs(lines, index, number, rows, result)) {
            return result;
        }
        if (!checkCaseExpect(lines, index, number, rows, result)) {
            return result;
        }
    }
    result.ok = true;
    return result;
}

}  // namespace aih
