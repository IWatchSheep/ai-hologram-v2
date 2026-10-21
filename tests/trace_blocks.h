// Purpose: Declares the blocks of a golden trace or observer
// file and the first-difference message of the gate (plan 3.14).
// Introduced in: Stage 3g (rebuild)
// Architecture: The gate compares the trace it writes for each
// case with the golden block of the same name. These helpers
// split a golden file into its blocks and, when two blocks
// differ, name the case, the decision step and the line of the
// first difference, as M1.4 requires.

#ifndef AI_HOLOGRAM_TESTS_TRACE_BLOCKS_H
#define AI_HOLOGRAM_TESTS_TRACE_BLOCKS_H

#include <string>
#include <vector>

namespace aih {

// One golden block: its case name and lines from "case" to "end".
struct TraceBlock {
    std::string name;
    std::vector<std::string> lines;
};

// splitTraceBlocks result: on failure blocks is empty and
// problem says exactly what was wrong.
struct TraceBlockSplit {
    bool ok = false;
    std::vector<TraceBlock> blocks;
    std::string problem;
};

// Splits a golden trace or observer file into its case blocks.
TraceBlockSplit splitTraceBlocks(const std::vector<std::string>& lines);

// "" when equal, or the first difference named by case, step
// (the last step or final line before it, or start) and line.
std::string firstTraceDifference(const std::string& name,
                                 const std::vector<std::string>& expected,
                                 const std::vector<std::string>& actual);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_TRACE_BLOCKS_H
