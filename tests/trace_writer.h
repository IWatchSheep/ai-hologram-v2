// Purpose: Declares the trace writer of the equivalence
// gate: it builds an agent from a case, runs it, and writes
// its trace or its observer lines (plan 3.14).
// Introduced in: Stage 3g (rebuild)
// Architecture: The writer runs the real agents through the
// real run loop, with only the loop's watch-only watchers
// attached (rule R6), so the traced run is the run itself.
// Comparing every decision step, not only the outcome, is
// what M1.4 asks for: a run could reach the right outcome
// by a different route and still not be the same agent.

#ifndef AI_HOLOGRAM_TESTS_TRACE_WRITER_H
#define AI_HOLOGRAM_TESTS_TRACE_WRITER_H

#include <string>
#include <vector>

#include "trace_cases.h"

namespace aih {

// Writes the full trace of one case (C1 or C0), from its
// "case" line to its "end" line, exactly as ref_trace.py.
std::vector<std::string> writeTrace(const TraceCase& traceCase);

// Writes the observer block of one case: the same case,
// step, final, result and end lines, with one observer line
// after each step or final, counting the IE-1 reports since
// the previous observer line.
std::vector<std::string> writeObserverTrace(const TraceCase& traceCase);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_TRACE_WRITER_H
