// Purpose: Implements investigateMismatch (plan 3.7): why a
// prediction disagreed with what was observed.
// Introduced in: Stage 3d (rebuild)
// Architecture: The candidates come from the footprint of
// the action, so only the few cells the action could reach
// are questioned (plan 9). The candidates are verified
// together: first against the recorded history, then by one
// variant prediction from the corrected belief. A variant
// prediction is used because the corrected belief is
// hypothetical: the result must not fill the predictor's
// cache as if the agent's own model had made it.

#include "agent/investigation.h"

#include <cstddef>
#include <optional>
#include <vector>

#include "agent/footprint.h"
#include "agent/history_checks.h"
#include "agent/observation.h"
#include "agent/records.h"
#include "agent/step_predictor.h"
#include "model/prediction.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

// The candidates of plan 3.7 step 3, from the footprint of
// the action: a wall candidate wherever the belief's wall
// differs from the record before the action, and one object
// candidate when the believed or recorded object is in a
// footprint cell and they differ.
std::vector<BeliefCandidate> findCandidates(const WorldState& beliefBefore,
                                            Action action,
                                            const Observation& before,
                                            const Observation& after) {
    const std::vector<Position> cells =
        footprintAfter(beliefBefore, action, after);
    std::vector<BeliefCandidate> candidates;
    for (const Position cell : cells) {
        if (isWall(beliefBefore, cell) == isWall(before.state, cell)) {
            continue;
        }
        BeliefCandidate candidate;
        candidate.element.kind = ElementKind::Wall;
        candidate.element.cell = cell;
        candidate.blocked = isWall(before.state, cell);
        candidates.push_back(candidate);
    }
    bool inFootprint = false;
    for (const Position cell : cells) {
        if (beliefBefore.object == cell || before.state.object == cell) {
            inFootprint = true;
        }
    }
    if (inFootprint && beliefBefore.object != before.state.object) {
        BeliefCandidate candidate;
        candidate.element.kind = ElementKind::Object;
        candidate.position = before.state.object;
        candidates.push_back(candidate);
    }
    return candidates;
}

// The belief with every candidate applied: the corrected
// belief the candidates are verified together on.
WorldState correctedBelief(const WorldState& belief,
                           const std::vector<BeliefCandidate>& candidates) {
    WorldState corrected = belief;
    for (const BeliefCandidate& candidate : candidates) {
        if (candidate.element.kind == ElementKind::Wall) {
            if (isInsideGrid(candidate.element.cell)) {
                const std::size_t x =
                    static_cast<std::size_t>(candidate.element.cell.x);
                const std::size_t y =
                    static_cast<std::size_t>(candidate.element.cell.y);
                corrected.walls[y][x] = candidate.blocked;
            }
        } else {
            if (candidate.element.kind == ElementKind::Object) {
                corrected.object = candidate.position;
            }
        }
    }
    return corrected;
}

// True when every candidate agrees with the history: walls
// against the whole history, the object against the
// observation before the action (o_t-1) only, because the
// object moves and earlier entries may show it elsewhere.
bool historyChecksPass(const std::vector<BeliefCandidate>& candidates,
                       const std::vector<Observation>& history,
                       const Observation& before) {
    for (const BeliefCandidate& candidate : candidates) {
        if (candidate.element.kind == ElementKind::Wall) {
            if (!wallHistoryCheck(candidate.element.cell, candidate.blocked,
                                  history)) {
                return false;
            }
        }
        if (candidate.element.kind == ElementKind::Object) {
            if (!objectHistoryCheck(candidate.position, before)) {
                return false;
            }
        }
    }
    return true;
}

// Verifies the candidates together, in the reference's
// order: the history checks and the corrected belief's
// validity, then the reserve test, then one variant
// prediction with the current model from the corrected
// belief, compared with the observation on its footprint.
InvestigationOutcome verifyCandidates(
    const WorldState& beliefBefore, Action action,
    const std::vector<BeliefCandidate>& candidates,
    const std::vector<Observation>& history, const Observation& before,
    const Observation& after, StepPredictor& predictor,
    const ActionModel& currentModel, int reserve) {
    if (!historyChecksPass(candidates, history, before)) {
        return InvestigationOutcome::NoCandidatePassed;
    }
    const WorldState corrected = correctedBelief(beliefBefore, candidates);
    if (!isValidWorld(corrected)) {
        return InvestigationOutcome::NoCandidatePassed;
    }
    if (predictor.remaining() <= reserve) {
        return InvestigationOutcome::Deferred;
    }
    const std::optional<Prediction> predicted =
        predictor.predictVariant(corrected, action, currentModel);
    if (!predicted.has_value()) {
        return InvestigationOutcome::Deferred;
    }
    const std::vector<Position> cells = computeFootprint(
        corrected, action, after.state, observedOutcome(after));
    const FootprintComparison comparison =
        compareFootprint(predicted->state, predicted->outcome, after.state,
                         observedOutcome(after), cells);
    if (isFootprintMatch(comparison)) {
        return InvestigationOutcome::BeliefCandidatesConsistent;
    }
    return InvestigationOutcome::NoCandidatePassed;
}

}  // namespace

InvestigationResult investigateMismatch(const WorldState& beliefBefore,
                                        Action action,
                                        const std::vector<Observation>& history,
                                        StepPredictor& predictor,
                                        const ActionModel& currentModel,
                                        int reserve) {
    InvestigationResult result;
    if (history.size() < 2) {
        return result;
    }
    const Observation& before = history[history.size() - 2];
    const Observation& after = history[history.size() - 1];
    result.anomaly = beliefBefore.agent != before.state.agent;
    result.candidates = findCandidates(beliefBefore, action, before, after);
    if (result.candidates.empty()) {
        result.outcome = InvestigationOutcome::ModelSuspected;
        result.suspected = SourceLabel::S1;
        return result;
    }
    result.suspected = SourceLabel::S2;
    result.outcome =
        verifyCandidates(beliefBefore, action, result.candidates, history,
                         before, after, predictor, currentModel, reserve);
    if (result.outcome == InvestigationOutcome::NoCandidatePassed) {
        result.suspected = SourceLabel::S5;
    }
    return result;
}

}  // namespace aih
