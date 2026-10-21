// Purpose: Implements the equivalence gate, part 2, on
// supplied case and golden lines (plan 3.15).
// Introduced in: Stage 4g (rebuild)
// Architecture: The gate follows the order of the Stage 3
// gate (runTraceGate), reusing its block and difference
// helpers (rule R4): a missing or empty golden file, the
// fault reader's problem, the split problem, the exact
// required case count, the trace count, a case name out
// of order, and then the first differing line, named by
// case, step and line. When all 563 blocks are identical,
// the rebuild has replayed every recorded fault run of
// the original exactly, and M1.4 has passed in full.

#include "fault_gate.h"

#include <string>
#include <vector>

#include "fault_case_reader.h"
#include "fault_trace_writer.h"
#include "trace_blocks.h"

namespace aih {

std::string runFaultGate(const std::vector<std::string>& caseLines,
                         const std::vector<std::string>& goldenLines,
                         std::size_t requiredCases) {
    if (caseLines.empty() || goldenLines.empty()) {
        return "a golden file is missing or empty";
    }
    const FaultCaseRead cases = readFaultCases(caseLines);
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
        const FaultCase& faultCase = cases.cases[i];
        const TraceBlock& block = blocks.blocks[i];
        if (block.name != faultCase.base.name) {
            return "trace " + block.name + ": expected case " +
                   faultCase.base.name;
        }
        const std::string failure = firstTraceDifference(
            faultCase.base.name, block.lines, writeFaultTrace(faultCase));
        if (!failure.empty()) {
            return failure;
        }
    }
    return "";
}

}  // namespace aih
