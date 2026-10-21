// Purpose: Handles one investigation result: logs its
// lines, makes its corrections, updates the belief on the
// footprint, starts model V1 work, or defers (plan 3.9).
// Introduced in: Stage 3e (rebuild)
// Architecture: The investigation lines are written first,
// in candidate order, and only then is anything changed,
// exactly as the reference orders it. A deferred result
// changes nothing at all: it stores the work for the next
// step and leaves the belief as it was.

#include "agent/single_agent.h"

#include <vector>

#include "agent/agent_log.h"
#include "agent/footprint.h"
#include "agent/investigation.h"
#include "agent/model_v1.h"
#include "agent/observation.h"
#include "agent/records.h"
#include "world/world_state.h"

namespace aih {

void SingleAgent::handleInvestigation(
    const InvestigationResult& result, const WorldState& beliefBefore,
    Action action, const Observation& after, bool logLines,
    const std::vector<Observation>& historyUsed) {
    if (logLines) {
        logInvestigation(result, beliefBefore, action);
    }
    const std::vector<Position> cells =
        footprintAfter(beliefBefore, action, after);
    if (result.outcome == InvestigationOutcome::BeliefCandidatesConsistent) {
        createBeliefCorrections(result, beliefBefore);
        applyFootprintUpdate(state.belief, cells, after.state);
        return;
    }
    if (result.outcome == InvestigationOutcome::NoCandidatePassed) {
        state.log.push_back(unexplainedLine());
        applyFootprintUpdate(state.belief, cells, after.state);
        return;
    }
    if (result.outcome == InvestigationOutcome::ModelSuspected) {
        // The belief is updated at once (D-025); whether the
        // model really is wrong is settled by the V1 replays.
        applyFootprintUpdate(state.belief, cells, after.state);
        state.pendingV1 = startModelV1(action, historyUsed);
        return;
    }
    if (logLines) {
        state.log.push_back(investigationDeferredLine());
    }
    DeferredInvestigation deferred;
    deferred.has = true;
    deferred.beliefBefore = beliefBefore;
    deferred.action = action;
    deferred.history = historyUsed;
    state.deferred = deferred;
}

void SingleAgent::logInvestigation(const InvestigationResult& result,
                                   const WorldState& beliefBefore,
                                   Action action) {
    if (result.candidates.empty()) {
        if (result.outcome == InvestigationOutcome::ModelSuspected) {
            state.log.push_back(investigationModelLine(action));
        }
    }
    for (const BeliefCandidate& candidate : result.candidates) {
        if (candidate.element.kind == ElementKind::Wall) {
            const bool believed = isWall(beliefBefore, candidate.element.cell);
            state.log.push_back(investigationWallLine(
                candidate.element.cell, believed, candidate.blocked));
        } else {
            state.log.push_back(investigationObjectLine(beliefBefore.object,
                                                        candidate.position));
        }
    }
    if (result.anomaly) {
        state.log.push_back(anomalyAgentLine());
    }
}

void SingleAgent::createBeliefCorrections(const InvestigationResult& result,
                                          const WorldState& beliefBefore) {
    for (const BeliefCandidate& candidate : result.candidates) {
        CorrectionRecord record;
        record.element = candidate.element;
        record.newBlocked = candidate.blocked;
        record.oldBlocked = isWall(beliefBefore, candidate.element.cell);
        record.newPosition = candidate.position;
        record.oldPosition = beliefBefore.object;
        record.status = CorrectionStatus::Consistent;
        record.stepConsistent = state.decisionStep;
        CorrectionRecord& added = addRecord(record);
        applyCorrection(added);
        if (added.element.kind == ElementKind::Wall) {
            state.log.push_back(correctionWallLine(added.id, added.element.cell,
                                                   added.newBlocked));
        } else {
            state.log.push_back(
                correctionObjectLine(added.id, added.newPosition));
        }
    }
}

}  // namespace aih
