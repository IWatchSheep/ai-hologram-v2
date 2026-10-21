// Purpose: The active test: walking to a cell from which
// one move tests an old wall correction (plan 3.9).
// Introduced in: Stage 3e (rebuild)
// Architecture: Only Consistent wall corrections at least
// activeTestDelay steps old are tested, in record order.
// The walk is an ordinary budgeted search whose goal is a
// target cell and whose heuristic makes no heuristicCalls
// count and no observer report: it is the plain Manhattan
// distance to the nearest target, or infinite when the
// object is dead.

#include "agent/single_agent.h"

#include <algorithm>
#include <optional>
#include <utility>
#include <vector>

#include "agent/agent_log.h"
#include "agent/dead_squares.h"
#include "agent/heuristic.h"
#include "agent/planner.h"
#include "agent/records.h"
#include "agent/verification.h"
#include "model/prediction.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

bool SingleAgent::tryActiveTest(const DeadSquareMap& map, int reserve) {
    for (const CorrectionRecord& record : state.corrections) {
        if (record.status != CorrectionStatus::Consistent) {
            continue;
        }
        if (record.element.kind != ElementKind::Wall) {
            continue;
        }
        if (state.decisionStep - record.stepConsistent <
            state.settings.activeTestDelay) {
            continue;
        }
        if (tryTestFor(record, map, reserve)) {
            return true;
        }
    }
    return false;
}

bool SingleAgent::tryTestFor(const CorrectionRecord& record,
                             const DeadSquareMap& map, int reserve) {
    const std::vector<std::pair<Position, Action>> targets =
        activeTestTargets(state.belief, record.element.cell);
    if (targets.empty()) {
        return false;
    }
    auto goal = [&targets](const WorldState& state) {
        for (const std::pair<Position, Action>& target : targets) {
            if (state.agent == target.first) {
                return true;
            }
        }
        return false;
    };
    auto estimate = [this, &map, &targets](const WorldState& state) {
        if (objectDead(state, map)) {
            return kInfiniteHeuristic;
        }
        int best = kInfiniteHeuristic;
        for (const std::pair<Position, Action>& target : targets) {
            best = std::min(best, manhattan(state.agent, target.first));
        }
        return best;
    };
    const SearchResult result =
        search(state.belief, map, goal, estimate, state.predictor, reserve);
    if (result.kind != SearchKind::Found) {
        return false;
    }
    return finishActiveTest(record, targets, result, reserve);
}

bool SingleAgent::finishActiveTest(
    const CorrectionRecord& record,
    const std::vector<std::pair<Position, Action>>& targets,
    const SearchResult& result, int reserve) {
    const WorldState end =
        result.plan.empty() ? state.belief : result.plan.back().expected;
    for (const std::pair<Position, Action>& target : targets) {
        if (target.first != end.agent) {
            continue;
        }
        if (!state.predictor.isCached(end, target.second)) {
            if (state.predictor.remaining() <= reserve) {
                continue;
            }
        }
        const std::optional<Prediction> test =
            state.predictor.predict(end, target.second);
        if (!test.has_value()) {
            continue;
        }
        state.plan = result.plan;
        state.plan.push_back(PlanStep{target.second, test->state});
        state.planKind = PlanKind::Test;
        state.log.push_back(
            planActiveTestLine(record.id, static_cast<int>(state.plan.size())));
        return true;
    }
    return false;
}

}  // namespace aih
