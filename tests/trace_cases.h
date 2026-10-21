// Purpose: Declares the trace cases of the equivalence gate
// and the reader for the case file (plan 3.14).
// Introduced in: Stage 3g (rebuild)
// Architecture: The gate (M1.4) replays 789 runs recorded from
// the original code and compares every decision step, not only
// the outcome, because a run could reach the right outcome by
// a different route. readTraceCases turns the case file into
// these values; the word readers it uses live in
// trace_case_parts, and the golden blocks in trace_blocks.

#ifndef AI_HOLOGRAM_TESTS_TRACE_CASES_H
#define AI_HOLOGRAM_TESTS_TRACE_CASES_H

#include <optional>
#include <string>
#include <vector>

#include "agent/agent_types.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// One seeded wall correction: the cell and its new and old values.
struct TraceSeedWall {
    Position cell{0, 0};
    bool newBlocked = false;
    bool oldBlocked = false;
};

// Seeded model V1 work: the action and the recorded uses to replay.
struct TraceSeedV1 {
    Action action = Action::North;
    int count = 0;
};

// One case: agent, settings, true world, optional given belief
// (none means "belief true"), model words, not-dead cells, seeds.
struct TraceCase {
    std::string name;
    std::string agent;
    AgentSettings settings;
    WorldState world{};
    std::optional<WorldState> belief;
    std::vector<std::string> modelSpec;
    std::vector<Position> notDead;
    std::vector<TraceSeedWall> walls;
    std::optional<TraceSeedV1> v1;
    std::optional<Action> committed;
    std::optional<Action> plan;
};

// readTraceCases result: on failure cases is empty and
// problem says exactly what was wrong, in the reference's words.
struct TraceCaseRead {
    bool ok = false;
    std::vector<TraceCase> cases;
    std::string problem;
};

// Reads all cases in order. Every line's existence is checked
// before it is read, so malformed input is reported, never
// read out of range, and nothing throws.
TraceCaseRead readTraceCases(const std::vector<std::string>& lines);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_TRACE_CASES_H
