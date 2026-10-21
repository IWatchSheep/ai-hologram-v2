// Purpose: Implements the A* search loop of plan 3.6 and the
// helper that adds one entry to the open list.
// Introduced in: Stage 3c (rebuild)
// Architecture: The search works only over the agent's own
// belief. It obtains every successor state from the
// StepPredictor, never from the rules directly, so every state
// it examines (apart from the root) has been paid for with
// exactly one budgeted model evaluation. Expanding one state
// lives in planner_expand.cpp; the one plan builder lives in
// planner_fallback.cpp (rule R4).

#include "agent/planner.h"

#include <cstddef>
#include <functional>

#include "agent/dead_squares.h"
#include "agent/planner_internal.h"
#include "agent/step_predictor.h"
#include "world/action.h"
#include "world/world_state.h"

namespace aih {

void addEntry(SearchState& search, const DiscoveredState& state, int index) {
    QueueEntry entry;
    entry.f = state.bestG + state.h;
    entry.h = state.h;
    entry.entryNumber = search.nextEntryNumber;
    entry.stateIndex = index;
    entry.g = state.bestG;
    search.queue.push(entry);
    search.nextEntryNumber = search.nextEntryNumber + 1;
}

void discoverRoot(SearchState& search, const WorldState& root,
                  const std::function<int(const WorldState&)>& heuristic) {
    DiscoveredState rootState;
    rootState.state = root;
    rootState.bestG = 0;
    rootState.h = heuristic(root);
    rootState.parentIndex = -1;
    rootState.action = Action::North;
    rootState.discoveryNumber = 0;
    search.result.discovered.push_back(rootState);
    search.closed.push_back(false);
    addEntry(search, search.result.discovered[0], 0);
}

SearchResult search(const WorldState& root, const DeadSquareMap& map,
                    const std::function<bool(const WorldState&)>& goalTest,
                    const std::function<int(const WorldState&)>& heuristic,
                    StepPredictor& predictor, int reserve) {
    SearchState searchState;
    discoverRoot(searchState, root, heuristic);
    while (!searchState.queue.empty()) {
        const QueueEntry entry = searchState.queue.top();
        searchState.queue.pop();
        const std::size_t index = static_cast<std::size_t>(entry.stateIndex);
        // Skip outdated entries: a shorter path was found after
        // this entry was added, or the state is already closed.
        if (entry.g != searchState.result.discovered[index].bestG) {
            continue;
        }
        if (searchState.closed[index]) {
            continue;
        }
        // The goal is accepted when a state is popped, not when
        // it is generated. The root is popped first, so an
        // already-solved root is Found with an empty plan and
        // no evaluations at all.
        if (goalTest(searchState.result.discovered[index].state)) {
            searchState.result.kind = SearchKind::Found;
            searchState.result.plan =
                buildPlan(searchState.result, entry.stateIndex);
            return searchState.result;
        }
        // Close this state and expand it in kActionOrder.
        if (!expand(searchState, map, heuristic, predictor, reserve,
                    entry.stateIndex)) {
            // OutOfBudget, already set by trySuccessor.
            return searchState.result;
        }
    }
    // Nothing left to take from the queue.
    searchState.result.kind = SearchKind::Exhausted;
    return searchState.result;
}

}  // namespace aih
