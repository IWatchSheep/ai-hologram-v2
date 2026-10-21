// Purpose: Declares the A* planner over the agent's own belief
// and its fallback choice (plan 3.6).
// Introduced in: Stage 3c (rebuild)
// Architecture: The planner never creates a successor state
// itself. Every successor comes from StepPredictor::predict,
// which charges the step budget (protocol 10.7); the root is
// the only state examined that did not come from a budgeted
// evaluation, because the caller gives it. The queue entry,
// its order and the shared search state live in
// planner_internal.h, used only by the three planner files.

#ifndef AI_HOLOGRAM_SRC_AGENT_PLANNER_H
#define AI_HOLOGRAM_SRC_AGENT_PLANNER_H

#include <functional>
#include <vector>

#include "agent/dead_squares.h"
#include "agent/step_predictor.h"
#include "world/action.h"
#include "world/world_state.h"

namespace aih {

// One step of a plan: the action to take, and the state the
// planner expects afterwards (the state the predictor produced
// for it).
struct PlanStep {
    Action action = Action::North;
    WorldState expected{};
};

// How a search ended. Found: a goal state was taken from the
// queue. OutOfBudget: the reserve rule stopped the search.
// Exhausted: the queue emptied without reaching the goal.
enum class SearchKind { Found, OutOfBudget, Exhausted };

// Bookkeeping kept for every state the search discovers. A
// state is identified by (agent position, object position);
// the walls and destination are the belief's, shared by all
// discovered states.
struct DiscoveredState {
    WorldState state{};
    int bestG = 0;
    int h = 0;
    int parentIndex = -1;
    Action action = Action::North;
    int discoveryNumber = 0;
};

// The outcome of one search: how it ended, the plan to the
// goal when found, every discovered state, and how many known
// paths were improved or reopened along the way.
struct SearchResult {
    SearchKind kind = SearchKind::Exhausted;
    std::vector<PlanStep> plan;
    std::vector<DiscoveredState> discovered;
    int improvedPaths = 0;
    int reopenedStates = 0;
};

// The A* search of plan 3.6. The open list is a
// std::priority_queue of entries ordered by (f = g + h, h,
// entry number), smallest first; planner_internal.h explains
// that container and its comparator. The goal is accepted when
// a state is popped from the queue, not when it is generated.
//
// reserve is the number of evaluations that must always be
// kept back for the step's mandatory predictions. This search
// is optional work, so before every prediction that is not
// already cached it checks that spending one evaluation would
// still leave the reserve; if not, it returns OutOfBudget.
SearchResult search(const WorldState& root, const DeadSquareMap& map,
                    const std::function<bool(const WorldState&)>& goalTest,
                    const std::function<int(const WorldState&)>& heuristic,
                    StepPredictor& predictor, int reserve);

// After OutOfBudget or Exhausted: the plan to the discovered
// non-root state with h < kInfiniteHeuristic and the smallest
// (h, bestG, discovery number). An empty plan means there is
// no such state.
std::vector<PlanStep> fallback(const SearchResult& result);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_PLANNER_H
