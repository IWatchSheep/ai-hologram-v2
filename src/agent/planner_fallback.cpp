// Purpose: Implements the one plan builder and the fallback
// choice of plan 3.6.
// Introduced in: Stage 3c (rebuild)
// Architecture: buildPlan is the one place a plan is built
// from parent links (rule R4): search uses it for the goal it
// found, and fallback uses it for the best state discovered
// when no goal was reached within the budget. The reserve that
// stopped the search protects the step's mandatory predictions,
// so the fallback plan is only ever followed after its first
// action has been predicted and paid for separately.

#include "agent/planner_internal.h"

#include <cstddef>
#include <vector>

#include "agent/heuristic.h"
#include "agent/planner.h"
#include "world/action.h"

namespace aih {

std::vector<PlanStep> buildPlan(const SearchResult& result, int stateIndex) {
    // Follow parents back to the root, collecting each step,
    // then reverse them into root-to-state order. Each step is
    // (action, the discovered state's stored state).
    std::vector<PlanStep> reversed;
    int at = stateIndex;
    while (result.discovered[static_cast<std::size_t>(at)].parentIndex != -1) {
        const DiscoveredState& step =
            result.discovered[static_cast<std::size_t>(at)];
        PlanStep planStep;
        planStep.action = step.action;
        planStep.expected = step.state;
        reversed.push_back(planStep);
        at = step.parentIndex;
    }
    std::vector<PlanStep> plan;
    for (std::size_t i = reversed.size(); i > 0; i = i - 1) {
        plan.push_back(reversed[i - 1]);
    }
    return plan;
}

std::vector<PlanStep> fallback(const SearchResult& result) {
    // Choose the discovered non-root state with a finite h and
    // the smallest (h, bestG, discovery number), in that order.
    int best = -1;
    for (std::size_t i = 0; i < result.discovered.size(); i = i + 1) {
        const DiscoveredState& candidate = result.discovered[i];
        if (candidate.parentIndex == -1) {
            continue;
        }
        if (candidate.h >= kInfiniteHeuristic) {
            continue;
        }
        if (best < 0) {
            best = static_cast<int>(i);
            continue;
        }
        const DiscoveredState& chosen =
            result.discovered[static_cast<std::size_t>(best)];
        bool smaller = false;
        if (candidate.h != chosen.h) {
            smaller = candidate.h < chosen.h;
        } else {
            if (candidate.bestG != chosen.bestG) {
                smaller = candidate.bestG < chosen.bestG;
            } else {
                smaller = candidate.discoveryNumber < chosen.discoveryNumber;
            }
        }
        if (smaller) {
            best = static_cast<int>(i);
        }
    }
    if (best < 0) {
        return std::vector<PlanStep>{};
    }
    return buildPlan(result, best);
}

}  // namespace aih
