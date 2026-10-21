// Purpose: Declares the settings, decision and plan kind
// shared by C1 and the run loop (plan 3.9).
// Introduced in: Stage 3e (rebuild)
// Architecture: These are plain values. AgentSettings is
// given to the agent and to the run loop, Decision is what
// decide() returns, and PlanKind says what the current plan
// is for. Keeping them here lets the state, the agent and
// the run loop share them without including each other.

#ifndef AI_HOLOGRAM_SRC_AGENT_AGENT_TYPES_H
#define AI_HOLOGRAM_SRC_AGENT_AGENT_TYPES_H

#include <string>

#include "world/action.h"

namespace aih {

// The per-run settings of an agent (plan 3.9). The defaults
// are the demonstration settings of plan 3.12.
struct AgentSettings {
    int budgetPerStep = 200;
    int safeStopLimit = 5;
    int maxWorldSteps = 100;
    int activeTestDelay = 3;
};

// What one decision step decided: either an action to take,
// or a safe stop ("take no action") with its reason. A safe
// stop must always be possible, so decide() can always
// return one of these instead of an action it cannot pay for.
struct Decision {
    bool isAction = false;
    Action action = Action::North;
    std::string stopReason;
};

// What the agent's current plan is: nothing, a plan to the
// goal, a fallback plan toward the best state found, or a
// plan that tests a correction (the active test).
enum class PlanKind { None, Goal, Fallback, Test };

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_AGENT_TYPES_H
