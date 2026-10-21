// Purpose: Declares the footprint: the few cells one action
// can reach, and how a prediction is compared on those cells.
// Introduced in: Stage 3a (rebuild)
// Architecture: C1 checks only where its action reaches (the
// footprint), a few cells per step, not the whole board (plan
// 9). These helpers are the one copy of that rule (rule R4).

#ifndef AI_HOLOGRAM_SRC_AGENT_FOOTPRINT_H
#define AI_HOLOGRAM_SRC_AGENT_FOOTPRINT_H

#include <vector>

#include "agent/observation.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {

// How one footprint cell differs: the cell, whether each of
// the three values differs, and the six values themselves.
// Members are written one per line, as the style rules require.
struct FootprintCellDifference {
    Position cell{};
    bool wallDiffers = false;
    bool agentDiffers = false;
    bool objectDiffers = false;
    bool predictedWall = false;
    bool observedWall = false;
    bool predictedAgentHere = false;
    bool observedAgentHere = false;
    bool predictedObjectHere = false;
    bool observedObjectHere = false;
};

// The result of comparing on the footprint: whether the
// outcomes differ, and the cells that differ, in order.
struct FootprintComparison {
    bool outcomesDiffer = false;
    std::vector<FootprintCellDifference> cells;
};

// Returns c0, c1 and possibly c2 (plan 3.2): the agent's
// cell in beliefBefore, then one and two cells ahead.
// c2 is included if the believed object is at c1, or the
// observed outcome is Pushed, or the observed object is at c1.
std::vector<Position> computeFootprint(const WorldState& beliefBefore,
                                       Action action,
                                       const WorldState& observedAfter,
                                       MoveOutcome observedOutcome);

// Compares the predicted and observed states on cells only,
// keeping a difference record where a value differs. Cells
// outside the grid are walls (isWall).
FootprintComparison compareFootprint(const WorldState& predictedState,
                                     MoveOutcome predictedOutcome,
                                     const WorldState& observedState,
                                     MoveOutcome observedOutcome,
                                     const std::vector<Position>& cells);

// The footprint when two predictions are compared: c2 is
// included if either belief has the object at c1, or either
// outcome is Pushed.
std::vector<Position> computeFootprintForTwoPredictions(
    const WorldState& firstBelief, MoveOutcome firstOutcome,
    const WorldState& secondBelief, MoveOutcome secondOutcome, Action action);

// Copies the observed wall, agent and object into belief,
// for footprint cells inside the grid only, in order.
// Nothing outside the footprint changes.
void applyFootprintUpdate(WorldState& belief,
                          const std::vector<Position>& cells,
                          const WorldState& observedState);

// The outcome an observation reports, or Blocked when it has
// none. This is the one copy of that rule (rule R4).
MoveOutcome observedOutcome(const Observation& observation);

// The footprint of an action, using the outcome the
// observation after it reports. The one copy (rule R4).
std::vector<Position> footprintAfter(const WorldState& beliefBefore,
                                     Action action, const Observation& after);

// True only when the outcomes agree and no cell differs.
// This is the one match test (rule R4).
bool isFootprintMatch(const FootprintComparison& comparison);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_FOOTPRINT_H
