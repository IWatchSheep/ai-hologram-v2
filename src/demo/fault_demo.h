// Purpose: Declares the Stage 4 demonstration of the
// measuring instrument on demonstration material only.
// Introduced in: Stage 4f (rebuild)
// Architecture: The demonstration runs C1 and C0 with
// runWithFaults from src/experiment/ and writes the
// results and CSV lines to a caller-provided stream. It
// is not an experiment: its lines show the instrument
// working, they are not results and are never compared,
// and no pilot or confirmatory layout is used.

#ifndef AI_HOLOGRAM_SRC_DEMO_FAULT_DEMO_H
#define AI_HOLOGRAM_SRC_DEMO_FAULT_DEMO_H

#include <iosfwd>

namespace aih {

// Writes the Stage 4 demonstration section (68 lines, the
// first one empty): the demonstration layout, two runs'
// fault events, the C1 and C0 runs of runs 500000-2 and
// 500003-1, and their CSV lines.
void runFaultDemo(std::ostream& out);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_DEMO_FAULT_DEMO_H
