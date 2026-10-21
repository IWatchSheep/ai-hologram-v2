// Purpose: The main search for a plan to the goal, the
// consequence surprise after an exhausted search, and the
// fallback choice (plan 3.9).
// Introduced in: Stage 3e (rebuild)
// Architecture: The search runs on the belief with the
// plan 3.3 heuristic, through the budgeted predictor, with
// the reserve kept back. An exhausted search may reveal a
// consequence surprise: a full-state dead end (no claim) or
// a trap correction, which is tested by trap V2 at once.

#include "agent/single_agent.h"

#include <optional>

#include "agent/agent_log.h"
#include "agent/dead_squares.h"
#include "agent/heuristic.h"
#include "agent/investigation.h"
#include "agent/planner.h"
#include "agent/records.h"
#include "model/prediction.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

Decision SingleAgent::runMainSearch(const DeadSquareMap& map, int reserve) {
    auto goal = [](const WorldState& state) {
        return state.object == state.destination;
    };
    auto estimate = [this, &map](const WorldState& given) {
        return heuristic(given, map, this->state.freeCounts);
    };
    const SearchResult result =
        search(state.belief, map, goal, estimate, state.predictor, reserve);
    if (result.kind == SearchKind::Found) {
        if (result.plan.empty()) {
            return safeStop("belief shows goal reached");
        }
        state.plan = result.plan;
        state.planKind = PlanKind::Goal;
        state.log.push_back(
            planSearchLine(static_cast<int>(state.plan.size())));
    } else {
        if (result.kind == SearchKind::Exhausted) {
            handleExhausted(map);
        }
        const std::optional<Decision> stopped = chooseFallback(result);
        if (stopped.has_value()) {
            return stopped.value();
        }
    }
    const std::optional<Prediction> predicted =
        state.predictor.predict(state.belief, state.plan[0].action);
    if (!predicted.has_value()) {
        state.plan.clear();
        state.planKind = PlanKind::None;
        return safeStop("no budget for a committed prediction");
    }
    return commitAction();
}

void SingleAgent::handleExhausted(const DeadSquareMap& map) {
    const Position object = state.belief.object;
    const ConsequenceOutcome consequence =
        checkConsequence(state.belief, map, state.history, state.freeCounts);
    if (consequence == ConsequenceOutcome::FullStateDeadEnd) {
        state.log.push_back(consequenceSurpriseLine(object));
        state.log.push_back(fullStateDeadEndLine());
        return;
    }
    if (consequence == ConsequenceOutcome::Mismatch) {
        state.log.push_back(consequenceSurpriseLine(object));
        state.log.push_back(consequenceMismatchLine(object));
        CorrectionRecord record;
        record.element.kind = ElementKind::Trap;
        record.element.cell = object;
        record.status = CorrectionStatus::Consistent;
        record.stepConsistent = state.decisionStep;
        CorrectionRecord& added = addRecord(record);
        applyCorrection(added);
        state.log.push_back(correctionTrapLine(added.id, object));
        // The trap V2 test is attempted at once, in this
        // same step, not left waiting for the next (plan 3.9).
        attemptTrapV2(added.id, object);
    }
}

std::optional<Decision> SingleAgent::chooseFallback(
    const SearchResult& result) {
    const std::vector<PlanStep> best = fallback(result);
    if (best.empty()) {
        if (result.kind == SearchKind::OutOfBudget) {
            return safeStop("budget too small to plan");
        }
        return safeStop("no safe action");
    }
    state.plan = best;
    state.planKind = PlanKind::Fallback;
    state.log.push_back(planFallbackLine(static_cast<int>(state.plan.size())));
    return std::nullopt;
}

}  // namespace aih
