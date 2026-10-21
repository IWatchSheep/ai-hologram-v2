// Purpose: Implements applyTraceSeeds, moved unchanged
// from the private applySeeds of trace_writer.cpp.
// Introduced in: Stage 4g (rebuild)
// Architecture: This is the seeding the Stage 3 trace
// writer has always done, in the order walls, V1, plan,
// committed, through the shared seeding helpers of
// agent_test_fixtures (rule R4). Moving it, and nothing
// else, is what lets the fault trace writer seed a C1
// agent in exactly the same way (plan 3.15).

#include "trace_agents.h"

#include <vector>

#include "agent_test_fixtures.h"
#include "trace_cases.h"

namespace aih {

void applyTraceSeeds(SingleAgent& agent, const TraceCase& traceCase) {
    if (!traceCase.walls.empty()) {
        std::vector<WallSeed> seeds;
        for (const TraceSeedWall& wall : traceCase.walls) {
            seeds.push_back(
                WallSeed{wall.cell, wall.newBlocked, wall.oldBlocked});
        }
        seedWallCorrections(agent, seeds);
    }
    if (traceCase.v1.has_value()) {
        seedModelV1(agent, traceCase.world, traceCase.v1.value().action,
                    traceCase.v1.value().count);
    }
    if (traceCase.plan.has_value()) {
        seedPlan(agent, traceCase.plan.value());
    }
    if (traceCase.committed.has_value()) {
        seedCommitted(agent, traceCase.committed.value());
    }
}

}  // namespace aih
