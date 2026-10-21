// Purpose: Continues work left over from earlier steps: a
// waiting trap V2 test first, then model V1 replays, and
// attempts one trap V2 test (plan 3.9, step 4).
// Introduced in: Stage 3e (rebuild)
// Architecture: Pending work runs before any new choosing,
// so a correction it settles is already in place when the
// step plans. The trap V2 attempt fetches the one dead-
// square map object from the cache (plan 3.15 Q3), so an
// override applied during the same choice is seen.

#include "agent/single_agent.h"

#include "agent/agent_log.h"
#include "agent/dead_squares.h"
#include "agent/model_v1.h"
#include "agent/records.h"
#include "agent/verification.h"
#include "world/position.h"

namespace aih {

void SingleAgent::continuePendingWork() {
    if (state.waitingTrap.has) {
        attemptTrapV2(state.waitingTrap.correctionId, state.waitingTrap.cell);
    }
    if (!state.pendingV1.has_value()) {
        return;
    }
    const Action action = state.pendingV1.value().action;
    const V1Progress progress =
        continueModelV1(state.pendingV1.value(), state.history, state.predictor,
                        reserveFor(state.corrections));
    if (progress == V1Progress::Done) {
        state.pendingV1 = std::nullopt;
        CorrectionRecord record;
        record.element.kind = ElementKind::ActionModel;
        record.element.action = action;
        record.status = CorrectionStatus::Consistent;
        record.stepConsistent = state.decisionStep;
        CorrectionRecord& added = addRecord(record);
        applyCorrection(added);
        state.log.push_back(correctionModelLine(added.id, action));
        return;
    }
    if (progress == V1Progress::Failed) {
        state.pendingV1 = std::nullopt;
        state.log.push_back(unexplainedLine());
    }
}

void SingleAgent::attemptTrapV2(int correctionId, Position cell) {
    CorrectionRecord* record = findCorrection(correctionId);
    if (record == nullptr) {
        state.waitingTrap = WaitingTrapTest{};
        return;
    }
    if (record->status != CorrectionStatus::Consistent) {
        state.waitingTrap = WaitingTrapTest{};
        return;
    }
    const int anomaliesBefore = state.trapAnomalies;
    // A reference to the cache's one map object (Q3): the
    // same object is recomputed into, never replaced.
    const DeadSquareMap& map =
        state.deadCache.get(state.belief, state.notDeadOverrides,
                            state.deadOverrides, state.freeCounts);
    const TrapV2Result result = trapV2Test(
        state.belief, map, cell, state.predictor, state.agentModel.current(),
        reserveFor(state.corrections), state.trapAnomalies);
    for (int index = anomaliesBefore; index < state.trapAnomalies;
         index = index + 1) {
        state.log.push_back(anomalyInvalidLine());
    }
    settleTrapV2(record, result, correctionId, cell);
}

void SingleAgent::settleTrapV2(CorrectionRecord* record, TrapV2Result result,
                               int correctionId, Position cell) {
    if (result == TrapV2Result::Verified) {
        record->status = CorrectionStatus::Verified;
        state.log.push_back(correctionVerifiedLine(correctionId));
        state.waitingTrap = WaitingTrapTest{};
        return;
    }
    if (result == TrapV2Result::Rejected) {
        record->status = CorrectionStatus::Rejected;
        state.log.push_back(correctionRejectedLine(correctionId));
        revertCorrection(*record);
        state.waitingTrap = WaitingTrapTest{};
        return;
    }
    WaitingTrapTest waiting;
    waiting.has = true;
    waiting.correctionId = correctionId;
    waiting.cell = cell;
    state.waitingTrap = waiting;
}

}  // namespace aih
