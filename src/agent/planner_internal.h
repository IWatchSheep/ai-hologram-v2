// Purpose: Declares what the three planner files share: the
// queue entry and its order, the search state, and the helper
// functions they use between them.
// Introduced in: Stage 3c (rebuild)
// Architecture: This header is used only by planner.cpp,
// planner_expand.cpp and planner_fallback.cpp (plan 4). The
// open list is a std::priority_queue: a standard container
// that always gives back the "largest" entry (according to its
// comparator) first. The planner wants the SMALLEST (f, h,
// entry number) first, so the comparator answers "does left
// belong BEHIND right?": yes when left's f is larger, or f ties
// and left's h is larger, or both tie and left was added later.
// Entry numbers count every push, so this order is fully
// deterministic even when f and h both tie.

#ifndef AI_HOLOGRAM_SRC_AGENT_PLANNER_INTERNAL_H
#define AI_HOLOGRAM_SRC_AGENT_PLANNER_INTERNAL_H

#include <functional>
#include <queue>
#include <vector>

#include "agent/dead_squares.h"
#include "agent/planner.h"
#include "agent/step_predictor.h"
#include "model/prediction.h"
#include "world/action.h"
#include "world/world_state.h"

namespace aih {

// One entry of the open list. g is the path length the entry
// was added with, so an older entry can be recognised as
// outdated after a shorter path to its state is found later.
struct QueueEntry {
    int f = 0;
    int h = 0;
    int entryNumber = 0;
    int stateIndex = 0;
    int g = 0;
};

// The open list's order: smallest f first, then smallest h,
// then smallest entry number. priority_queue puts the entry
// that is NOT behind any other first, so this comparator says
// left is behind right exactly in the cases listed above.
struct QueueEntryBehind {
    bool operator()(const QueueEntry& left, const QueueEntry& right) const {
        if (left.f != right.f) {
            return left.f > right.f;
        }
        if (left.h != right.h) {
            return left.h > right.h;
        }
        return left.entryNumber > right.entryNumber;
    }
};

// The open list type: a priority_queue of queue entries.
using OpenList =
    std::priority_queue<QueueEntry, std::vector<QueueEntry>, QueueEntryBehind>;

// The mutable bookkeeping of one running search, grouped so
// the helper functions each stay short and take few parameters.
struct SearchState {
    SearchResult result;
    OpenList queue;
    std::vector<bool> closed;
    int nextEntryNumber = 0;
};

// Adds one queue entry for a discovered state and counts it:
// entry numbers count every push.
void addEntry(SearchState& search, const DiscoveredState& state, int index);

// Discovers the root (g 0, h computed now, discovery number
// 0, no parent) and pushes its queue entry.
void discoverRoot(SearchState& search, const WorldState& root,
                  const std::function<int(const WorldState&)>& heuristic);

// Finds the already-discovered state with the same agent and
// object as state, or -1 when there is none.
int findDiscovered(const SearchResult& result, const WorldState& state);

// Builds the plan from the root to discovered[stateIndex] by
// following parent links backwards and reversing them. This
// is the one plan builder (rule R4), used by search and by
// fallback; it lives in planner_fallback.cpp.
std::vector<PlanStep> buildPlan(const SearchResult& result, int stateIndex);

// Records a newly discovered successor and queues it. Its h
// is computed now, at discovery time.
void discover(SearchState& search, const Prediction& prediction, int newG,
              int parentIndex, Action action,
              const std::function<int(const WorldState&)>& heuristic);

// Improves an already-discovered successor reached by a
// shorter path: counts it, reopens and counts it when it was
// closed, replaces bestG, parent and action, and queues a new
// entry. Its stored state and h are unchanged.
void improve(SearchState& search, int knownIndex, int newG, int parentIndex,
             Action action);

// Tries one action from the state being expanded. Returns
// false only when the search must stop with OutOfBudget.
bool trySuccessor(SearchState& search, const DeadSquareMap& map,
                  const std::function<int(const WorldState&)>& heuristic,
                  StepPredictor& predictor, int reserve, int currentIndex,
                  Action action);

// Closes and expands one taken state, trying each action in
// kActionOrder. Returns false only on OutOfBudget.
bool expand(SearchState& search, const DeadSquareMap& map,
            const std::function<int(const WorldState&)>& heuristic,
            StepPredictor& predictor, int reserve, int currentIndex);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_PLANNER_INTERNAL_H
