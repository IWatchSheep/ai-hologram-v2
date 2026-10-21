// Purpose: Checks a committed prediction against the new
// observation, settles its tested corrections, and retries
// a deferred investigation (plan 3.9, steps 2 and 3).
// Introduced in: Stage 3e (rebuild)
// Architecture: The check compares only on the footprint of
// the committed action, never the whole belief (Phase B
// 12.1). Tested corrections are settled in the order their
// ids were recorded at commit time, each Verified on a
// match or Rejected (and reverted) on a mismatch.

#include "agent/single_agent.h"

#include <optional>
#include <vector>

#include "agent/agent_log.h"
#include "agent/agent_types.h"
#include "agent/footprint.h"
#include "agent/investigation.h"
#include "agent/observation.h"
#include "agent/records.h"
#include "world/world_state.h"

namespace aih {

void SingleAgent::checkCommitted(const Observation& observation) {
    const CommittedPrediction committed = state.committed;
    state.committed = CommittedPrediction{};
    const WorldState& before = committed.beliefBefore;
    const Action action = committed.prediction.action;
    const std::vector<Position> cells =
        footprintAfter(before, action, observation);
    const FootprintComparison comparison = compareFootprint(
        committed.prediction.state, committed.prediction.outcome,
        observation.state, observedOutcome(observation), cells);
    const bool matched = isFootprintMatch(comparison);
    state.log.push_back(stepLine(committed.decisionStep, action,
                                 committed.prediction.outcome,
                                 observedOutcome(observation), matched));
    settleTested(committed, matched);
    if (matched) {
        applyFootprintUpdate(state.belief, cells, observation.state);
        return;
    }
    // The reserve is computed now, with the corrections as
    // they stand after settling, exactly as the reference
    // computes it (see single_agent_choose.cpp for why).
    const InvestigationResult result = investigateMismatch(
        before, action, state.history, state.predictor,
        state.agentModel.current(), reserveFor(state.corrections));
    MismatchRecord mismatch;
    mismatch.decisionStep = state.decisionStep;
    mismatch.action = action;
    mismatch.differences = comparison;
    for (const BeliefCandidate& candidate : result.candidates) {
        mismatch.candidates.push_back(candidate.element);
    }
    mismatch.suspected = result.suspected;
    mismatch.anomaly = result.anomaly;
    state.mismatches.push_back(mismatch);
    handleInvestigation(result, before, action, observation, true,
                        state.history);
}

void SingleAgent::settleTested(const CommittedPrediction& committed,
                               bool matched) {
    for (const int correctionId : committed.testedCorrectionIds) {
        CorrectionRecord* record = findCorrection(correctionId);
        if (record == nullptr) {
            continue;
        }
        if (record->status != CorrectionStatus::Consistent) {
            continue;
        }
        if (matched) {
            record->status = CorrectionStatus::Verified;
            state.log.push_back(correctionVerifiedLine(correctionId));
        } else {
            record->status = CorrectionStatus::Rejected;
            state.log.push_back(correctionRejectedLine(correctionId));
            revertCorrection(*record);
        }
    }
}

std::optional<Decision> SingleAgent::retryDeferred() {
    if (!state.deferred.has) {
        return std::nullopt;
    }
    const DeferredInvestigation deferred = state.deferred;
    const InvestigationResult result = investigateMismatch(
        deferred.beliefBefore, deferred.action, deferred.history,
        state.predictor, state.agentModel.current(),
        reserveFor(state.corrections));
    if (result.outcome == InvestigationOutcome::Deferred) {
        return safeStop("budget too small for verification");
    }
    state.deferred = DeferredInvestigation{};
    // Retried work is handled without logging its
    // investigation lines again (plan 3.9), with the stored
    // history and its last entry as the observation after.
    handleInvestigation(result, deferred.beliefBefore, deferred.action,
                        deferred.history.back(), false, deferred.history);
    return std::nullopt;
}

}  // namespace aih
