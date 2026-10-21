// Purpose: Declares the shared, bounds-safe golden-file
// readers and the result they return.
// Introduced in: Stage 1b (rebuild)
// Architecture: Both golden tests and the malformed-file
// tests use these readers (rule R4). Each reader takes a
// file's lines, checks that a line exists before reading
// it, never reads out of range and never throws; it follows
// tools/reference/stage1/ref_golden_readers.py exactly.

#ifndef AI_HOLOGRAM_TESTS_GOLDEN_READERS_H
#define AI_HOLOGRAM_TESTS_GOLDEN_READERS_H

#include <string>
#include <vector>

namespace aih {

// What checking a whole golden file found: ok is true only
// when every case matched, cases counts the cases checked,
// and problem is empty when ok, otherwise it holds the one
// exact problem message of the reference reader.
struct GoldenCheck {
    bool ok = false;
    int cases = 0;
    std::string problem;
};

// Checks world_parse_golden.txt lines, one case at a time:
// a "case <n> lines <k>" header, k input lines and one
// "expect " line, compared with parsing the inputs again.
GoldenCheck checkParseGoldenLines(const std::vector<std::string>& lines);

// Checks world_rules_golden.txt lines: each "layout <id>"
// gives 10 lines, and each case line is rebuilt by applying
// its action to its layout and compared in full.
GoldenCheck checkRulesGoldenLines(const std::vector<std::string>& lines);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_GOLDEN_READERS_H
