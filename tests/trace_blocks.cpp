// Purpose: Implements splitting a golden file into case blocks
// and naming the first difference between two traces (plan 3.14).
// Introduced in: Stage 3g (rebuild)
// Architecture: Both follow ref_trace_reader.py exactly. The
// splitter checks each header before reading it, so a malformed
// file gives a problem text instead of reading past the end.

#include "trace_blocks.h"

#include <algorithm>
#include <string>
#include <vector>

#include "golden_files.h"

namespace aih {

TraceBlockSplit splitTraceBlocks(const std::vector<std::string>& lines) {
    TraceBlockSplit result;
    int index = 0;
    const int size = static_cast<int>(lines.size());
    while (index < size) {
        const std::vector<std::string> words = splitWords(lines[index]);
        if (words.size() != 2 || words[0] != "case") {
            result.problem = "line " + std::to_string(index + 1) +
                             ": expected a case header";
            result.blocks.clear();
            return result;
        }
        const int start = index;
        while (index < size && lines[index] != "end") {
            index = index + 1;
        }
        if (index >= size) {
            result.problem = "trace " + words[1] + ": missing end line";
            result.blocks.clear();
            return result;
        }
        TraceBlock block;
        block.name = words[1];
        block.lines.assign(lines.begin() + start, lines.begin() + index + 1);
        result.blocks.push_back(block);
        index = index + 1;
    }
    result.ok = true;
    return result;
}

std::string firstTraceDifference(const std::string& name,
                                 const std::vector<std::string>& expected,
                                 const std::vector<std::string>& actual) {
    std::string step = "start";
    const std::size_t count = std::max(expected.size(), actual.size());
    for (std::size_t k = 0; k < count; k = k + 1) {
        const std::string wanted =
            k < expected.size() ? expected[k] : "<missing>";
        const std::string got = k < actual.size() ? actual[k] : "<missing>";
        if (wanted != got) {
            return "case " + name + ", " + step + ", line " +
                   std::to_string(k + 1) + ": expected [" + wanted +
                   "] actual [" + got + "]";
        }
        if (startsWith(wanted, "step ") || wanted == "final") {
            step = wanted;
        }
    }
    return "";
}

}  // namespace aih
