// Purpose: Declares the fault trace writer of the
// equivalence gate, part 2: it runs one fault case with
// runWithFaults and writes its fault trace.
// Introduced in: Stage 4g (rebuild)
// Architecture: The writer builds the agent from the case
// exactly as the Stage 3 trace writer does (and seeds C1
// with applyTraceSeeds, rule R4), then runs the real run
// with faults with observe true. Its extra watchers only
// watch (rule R6): after every decision step they write
// that step's Stage 3 trace lines, then every activation
// and score line whose text changed since it was last
// printed. Comparing these traces line by line with the
// golden ones is what proves the rebuild does exactly
// what the original code did (M1.4).

#ifndef AI_HOLOGRAM_TESTS_FAULT_TRACE_WRITER_H
#define AI_HOLOGRAM_TESTS_FAULT_TRACE_WRITER_H

#include <string>
#include <vector>

#include "fault_case_reader.h"

namespace aih {

// Writes the full fault trace of one case (C1 or C0), from
// its "case" line to its "end" line, exactly as
// ref_fault_trace.trace writes it.
std::vector<std::string> writeFaultTrace(const FaultCase& faultCase);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_FAULT_TRACE_WRITER_H
