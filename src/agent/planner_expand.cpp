// Purpose: Implements expanding one state of the A* search:
// finding, discovering and improving states, and trying one
// successor through the step predictor.
// Introduced in: Stage 3c (rebuild)
// Architecture: Every successor state comes from the step
// predictor, never from the rules directly. Making one any
// other way would be an uncharged evaluation, which protocol
// 10.7 forbids; this is why the planner never calls the rules
// itself. The reserve protects the step's mandatory
// predictions: a cached prediction costs nothing, so only an
// uncached one must show it would still leave the reserve.

#include "agent/planner_internal.h"

#include <cstddef>
#include <functional>
#include <optional>

#include "agent/dead_squares.h"
#include "agent/directions.h"
#include "agent/planner.h"
#include "agent/step_predictor.h"
#include "model/prediction.h"
#include "world/action.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

// True when a predicted result goes nowhere and is skipped:
// Blocked and invalid results, and a state whose object is on
// a dead square (read through readDeadStatus, IE-1 H4), which
// can never reach the goal. Skipped, but the search goes on.
bool isSkippedResult(const Prediction& prediction, const DeadSquareMap& map) {
    if (prediction.outcome == MoveOutcome::Blocked) {
        return true;
    }
    if (prediction.outcome == MoveOutcome::InvalidState) {
        return true;
    }
    return readDeadStatus(map, prediction.state.object);
}

}  // namespace

int findDiscovered(const SearchResult& result, const WorldState& state) {
    for (std::size_t i = 0; i < result.discovered.size(); i = i + 1) {
        const WorldState& known = result.discovered[i].state;
        if (known.agent == state.agent) {
            if (known.object == state.object) {
                return static_cast<int>(i);
            }
        }
    }
    return -1;
}

void discover(SearchState& search, const Prediction& prediction, int newG,
              int parentIndex, Action action,
              const std::function<int(const WorldState&)>& heuristic) {
    DiscoveredState fresh;
    fresh.state = prediction.state;
    fresh.bestG = newG;
    fresh.h = heuristic(prediction.state);
    fresh.parentIndex = parentIndex;
    fresh.action = action;
    fresh.discoveryNumber = static_cast<int>(search.result.discovered.size());
    search.result.discovered.push_back(fresh);
    search.closed.push_back(false);
    const int index = static_cast<int>(search.result.discovered.size()) - 1;
    addEntry(search, fresh, index);
}

void improve(SearchState& search, int knownIndex, int newG, int parentIndex,
             Action action) {
    const std::size_t known = static_cast<std::size_t>(knownIndex);
    search.result.improvedPaths = search.result.improvedPaths + 1;
    if (search.closed[known]) {
        search.closed[known] = false;
        search.result.reopenedStates = search.result.reopenedStates + 1;
    }
    DiscoveredState& state = search.result.discovered[known];
    state.bestG = newG;
    state.parentIndex = parentIndex;
    state.action = action;
    addEntry(search, state, knownIndex);
}

bool trySuccessor(SearchState& search, const DeadSquareMap& map,
                  const std::function<int(const WorldState&)>& heuristic,
                  StepPredictor& predictor, int reserve, int currentIndex,
                  Action action) {
    const std::size_t current = static_cast<std::size_t>(currentIndex);
    const WorldState state = search.result.discovered[current].state;
    const int newG = search.result.discovered[current].bestG + 1;
    // The reserve test: an uncached prediction costs one, so
    // it may only be bought when more than the reserve remains.
    if (!predictor.isCached(state, action)) {
        if (predictor.remaining() <= reserve) {
            search.result.kind = SearchKind::OutOfBudget;
            return false;
        }
    }
    const std::optional<Prediction> prediction =
        predictor.predict(state, action);
    if (!prediction.has_value()) {
        search.result.kind = SearchKind::OutOfBudget;
        return false;
    }
    if (isSkippedResult(*prediction, map)) {
        return true;
    }
    const int knownIndex = findDiscovered(search.result, prediction->state);
    if (knownIndex >= 0) {
        const std::size_t known = static_cast<std::size_t>(knownIndex);
        if (newG >= search.result.discovered[known].bestG) {
            return true;
        }
        improve(search, knownIndex, newG, currentIndex, action);
        return true;
    }
    discover(search, *prediction, newG, currentIndex, action, heuristic);
    return true;
}

bool expand(SearchState& search, const DeadSquareMap& map,
            const std::function<int(const WorldState&)>& heuristic,
            StepPredictor& predictor, int reserve, int currentIndex) {
    const std::size_t current = static_cast<std::size_t>(currentIndex);
    search.closed[current] = true;
    for (const Action action : kActionOrder) {
        if (!trySuccessor(search, map, heuristic, predictor, reserve,
                          currentIndex, action)) {
            return false;
        }
    }
    return true;
}

}  // namespace aih
