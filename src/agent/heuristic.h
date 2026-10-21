// Purpose: Declares the planner's heuristic estimate and the
// one Manhattan distance helper.
// Introduced in: Stage 3b (rebuild)
// Architecture: The heuristic is a free but bounded
// computation (protocol 10.7): one table lookup in the
// DeadSquareMap plus Manhattan distances. It does no search,
// loops over no actions and never calls any model, so it can
// never create a successor state and is never charged to a
// BudgetCounter. It is called only on states that already
// exist (the belief, or states produced by budgeted model
// evaluations).

#ifndef AI_HOLOGRAM_SRC_AGENT_HEURISTIC_H
#define AI_HOLOGRAM_SRC_AGENT_HEURISTIC_H

#include "agent/dead_squares.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// A value far larger than any real plan length on a 10x10
// grid. It means "unreachable": the object is on a believed
// dead square, from which it can never reach the destination.
constexpr int kInfiniteHeuristic = 1000000000;

// The Manhattan distance between two cells: the steps when
// only the four directions are allowed and walls are ignored.
// This is the one copy of that helper (rule R4).
int manhattan(Position first, Position second);

// Estimates the actions still needed from state (plan 3.3):
// kInfiniteHeuristic when the object's cell is dead, and
// otherwise P + max(0, manhattan(agent, object) - 1), where
// P is the object's pushDistance when it is at least 0, and
// manhattan(object, destination) when it is -1, which happens
// only on an overridden cell. It adds 1 to heuristicCalls.
int heuristic(const WorldState& state, const DeadSquareMap& map,
              FreeComputationCounts& counts);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_HEURISTIC_H
