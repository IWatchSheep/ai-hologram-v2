// Purpose: Chooses and commits the step's action: the
// cautious mode, keeping a plan, and committing itself,
// with its V2 bookkeeping (plan 3.9, step 6).
// Introduced in: Stage 3e (rebuild)
// Architecture: The reserve (reserveFor, the one rule) is
// the number of evaluations that must always be kept back
// for the step's mandatory predictions, so optional work -
// searches, tests, V2 alternatives - never spends the
// budget the committed prediction itself needs. Committing
// predicts the plan's first action through the step
// predictor like every other prediction.

#include "agent/single_agent.h"

#include <optional>
#include <vector>

#include "agent/agent_log.h"
#include "agent/dead_squares.h"
#include "agent/directions.h"
#include "agent/planner.h"
#include "agent/records.h"
#include "agent/verification.h"
#include "model/prediction.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {

std::optional<PlanStep> SingleAgent::chooseCautiousAction() {
    // While model V1 work is unfinished, only an action the
    // current model and the standard rule agree on, landing
    // the object on a cell that is not dead, may be taken.
    const DeadSquareMap& map =
        state.deadCache.get(state.belief, state.notDeadOverrides,
                            state.deadOverrides, state.freeCounts);
    const int reserve = reserveFor(state.corrections);
    for (const Action action : kActionOrder) {
        const int need = state.predictor.isCached(state.belief, action) ? 1 : 2;
        if (state.predictor.remaining() - need < reserve) {
            break;
        }
        const std::optional<Prediction> current =
            state.predictor.predict(state.belief, action);
        if (!current.has_value()) {
            break;
        }
        const std::optional<Prediction> standard =
            state.predictor.predictVariant(state.belief, action, applyAction);
        if (!standard.has_value()) {
            break;
        }
        if (!predictionsDiffer(current.value(), standard.value())) {
            if (!objectDead(current->state, map)) {
                return PlanStep{action, current->state};
            }
        }
    }
    return std::nullopt;
}

Decision SingleAgent::chooseAndCommit() {
    if (state.pendingV1.has_value()) {
        const std::optional<PlanStep> cautious = chooseCautiousAction();
        if (cautious.has_value()) {
            state.log.push_back(planCautiousLine());
            state.plan = {cautious.value()};
            state.planKind = PlanKind::Goal;
            return commitAction();
        }
        return safeStop("investigation unfinished, no cautious action");
    }
    const int reserve = reserveFor(state.corrections);
    // One map object for the whole choice (plan 3.15 Q3):
    // a trap override applied during it is seen through it.
    const DeadSquareMap& map =
        state.deadCache.get(state.belief, state.notDeadOverrides,
                            state.deadOverrides, state.freeCounts);
    if (state.planKind != PlanKind::Test) {
        if (tryActiveTest(map, reserve)) {
            return commitAction();
        }
    }
    const std::optional<Decision> kept = tryKeepPlan(map);
    if (kept.has_value()) {
        return kept.value();
    }
    return runMainSearch(map, reserve);
}

std::optional<Decision> SingleAgent::tryKeepPlan(const DeadSquareMap& map) {
    if (state.plan.empty()) {
        return std::nullopt;
    }
    const std::optional<Prediction> predicted =
        state.predictor.predict(state.belief, state.plan[0].action);
    if (!predicted.has_value()) {
        state.plan.clear();
        state.planKind = PlanKind::None;
        return safeStop("no budget for a committed prediction");
    }
    if (predicted->state == state.plan[0].expected) {
        if (!objectDead(predicted->state, map)) {
            return commitAction();
        }
    }
    state.plan.clear();
    state.planKind = PlanKind::None;
    return std::nullopt;
}

Decision SingleAgent::commitAction() {
    const Action action = state.plan[0].action;
    const std::optional<Prediction> predicted =
        state.predictor.predict(state.belief, action);
    if (!predicted.has_value()) {
        state.plan.clear();
        state.planKind = PlanKind::None;
        return safeStop("no budget for a committed prediction");
    }
    state.plan.erase(state.plan.begin());
    if (state.plan.empty()) {
        state.planKind = PlanKind::None;
    }
    const std::vector<int> tested =
        collectTestedCorrections(action, predicted.value());
    CommittedPrediction committed;
    committed.has = true;
    committed.prediction = predicted.value();
    committed.beliefBefore = state.belief;
    committed.decisionStep = state.decisionStep;
    committed.testedCorrectionIds = tested;
    state.committed = committed;
    return Decision{true, action, ""};
}

std::vector<int> SingleAgent::collectTestedCorrections(
    Action action, const Prediction& predicted) {
    // V2 bookkeeping: a Consistent, untested correction is
    // tested when undoing it would predict something
    // different for this very action (plan 3.9). Only then
    // will the next step's check settle it.
    std::vector<int> tested;
    for (CorrectionRecord& record : state.corrections) {
        if (record.status != CorrectionStatus::Consistent) {
            continue;
        }
        if (record.tested) {
            continue;
        }
        if (state.predictor.remaining() <= 0) {
            continue;
        }
        const std::optional<Prediction> alternative = alternativePrediction(
            record, state.belief, action, state.predictor,
            state.agentModel.current(), state.agentModel.base());
        if (alternative.has_value()) {
            if (predictionsDiffer(alternative.value(), predicted)) {
                record.tested = true;
                tested.push_back(record.id);
            }
        }
    }
    return tested;
}

}  // namespace aih
