// Purpose: Declares the two C1 demonstrations of plan 3.12.
// Introduced in: Stage 3f (rebuild)
// Architecture: The demonstrations run the real C1 through
// the real run loop and write its lines to a caller-provided
// stream, so the program and the test can both receive the
// output where they need it.

#ifndef AI_HOLOGRAM_SRC_DEMO_AGENT_DEMO_H
#define AI_HOLOGRAM_SRC_DEMO_AGENT_DEMO_H

#include <iosfwd>

namespace aih {

// Runs the two single-agent demonstrations, writing to out.
// Demo 2's wrong belief is hand-written; it is not fault
// injection (plan 3.12).
void runAgentDemos(std::ostream& out);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_DEMO_AGENT_DEMO_H
