// Purpose: Declares what the run loop delivers to an agent
// at each decision step: the true state and what just happened.
// Introduced in: Stage 3a (rebuild)
// Architecture: The run loop builds every observation from
// the Environment only. C1 keeps every observation in its
// history, in order, and nothing ever changes an entry.

#ifndef AI_HOLOGRAM_SRC_AGENT_OBSERVATION_H
#define AI_HOLOGRAM_SRC_AGENT_OBSERVATION_H

#include <optional>

#include "world/action.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {

// One observation, exactly as plan 3.1 writes it. Members
// are written one per line, as the style rules require.
struct Observation {
    int decisionStep = 0;
    WorldState state{};
    std::optional<Action> lastAction;
    std::optional<MoveOutcome> lastOutcome;
};

// std::optional holds a value or nothing at all. lastAction
// and lastOutcome are empty at the start of a run and after
// a safe stop, because no action was taken to report.

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_OBSERVATION_H
