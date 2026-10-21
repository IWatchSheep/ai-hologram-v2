// Purpose: Declares applyTraceSeeds, the one way a built
// C1 agent is seeded from its trace case (rule R4).
// Introduced in: Stage 4g (rebuild)
// Architecture: The Stage 3 trace writer and the Stage 4
// fault trace writer both run C1 agents built from cases,
// so they must seed those agents in exactly the same way.
// The seeding lived as a private helper of the Stage 3
// trace writer; it moves here unchanged so there is only
// one copy of it, and both writers use it.

#ifndef AI_HOLOGRAM_TESTS_TRACE_AGENTS_H
#define AI_HOLOGRAM_TESTS_TRACE_AGENTS_H

#include "agent/single_agent.h"
#include "trace_cases.h"

namespace aih {

// Seeds a built C1 agent from its case, in the order of
// plan 3.14, through the shared seeding helpers of
// agent_test_fixtures (rule R4): walls, V1, plan, committed.
void applyTraceSeeds(SingleAgent& agent, const TraceCase& traceCase);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_TRACE_AGENTS_H
