// Purpose: Implements the equivalence gate on supplied case
// and golden lines (plan 3.14).
// Introduced in: Stage 3g (rebuild), correction of plan v1.6
// Architecture: This is the gate logic that used to live in
// checkGate in test_trace_gate.cpp, moved as it was, plus the
// one new required-count check. The exact count matters for
// coverage: if both files were truncated in the same way,
// every remaining case would still match, so without the
// count a shortened gate could pass while testing less.

#include "trace_gate.h"

#include <string>
#include <vector>

#include "trace_blocks.h"
#include "trace_cases.h"
#include "trace_writer.h"

namespace aih {

std::string runTraceGate(const std::vector<std::string>& caseLines,
                         const std::vector<std::string>& goldenLines,
                         bool observers, std::size_t requiredCases) {
    if (caseLines.empty() || goldenLines.empty()) {
        return "a golden file is missing or empty";
    }
    const TraceCaseRead cases = readTraceCases(caseLines);
    if (!cases.ok) {
        return cases.problem;
    }
    const TraceBlockSplit blocks = splitTraceBlocks(goldenLines);
    if (!blocks.ok) {
        return blocks.problem;
    }
    if (cases.cases.size() != requiredCases) {
        return "case file has " + std::to_string(cases.cases.size()) +
               " cases, required " + std::to_string(requiredCases);
    }
    if (blocks.blocks.size() != cases.cases.size()) {
        return "trace file has " + std::to_string(blocks.blocks.size()) +
               " cases, the case file " + std::to_string(cases.cases.size());
    }
    for (std::size_t i = 0; i < cases.cases.size(); i = i + 1) {
        const TraceCase& traceCase = cases.cases[i];
        const TraceBlock& block = blocks.blocks[i];
        if (block.name != traceCase.name) {
            return "trace " + block.name + ": expected case " + traceCase.name;
        }
        const std::vector<std::string> actual =
            observers ? writeObserverTrace(traceCase) : writeTrace(traceCase);
        const std::string failure =
            firstTraceDifference(traceCase.name, block.lines, actual);
        if (!failure.empty()) {
            return failure;
        }
    }
    return "";
}

}  // namespace aih
