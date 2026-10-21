// Purpose: Implements afterDecide and the committed-
// prediction shadow: the shadow of each committed
// prediction, and justified safe stops (plan 3.7, 3.9).
// Introduced in: Stage 4d (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. A shadow repeats the committed
// prediction with one faulted element at its true value,
// using the Stage 3 footprint helpers (rule R4). Shadow
// work calls the reasoner's current model directly, so it
// never uses a budget and is never seen by an agent. When
// the shadow gives a different result, the fault made a
// difference in a prediction the agent committed to: the
// event is a committed cause and the use is revealing.

#include "experiment/scorer.h"

#include <cstddef>
#include <optional>
#include <vector>

#include "agent/footprint.h"
#include "experiment/shadow.h"
#include "model/world_model.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {

bool Scorer::committedF3Effective(const FaultEvent& event,
                                  const WorldState& pre, Action action,
                                  const WorldState& resultState,
                                  MoveOutcome resultOutcome,
                                  const ScoredView& view) const {
    if (action != event.action) {
        return false;
    }
    if (view.model == nullptr || view.model->isRestored(action)) {
        return false;
    }
    const ActionResult standard = applyAction(pre, action);
    return standard.outcome != resultOutcome || standard.state != resultState;
}

Position Scorer::committedTrueObject(const WorldState& pre,
                                     const ScoredView& view) const {
    Position trueObject = pre.object;
    if (view.belief != nullptr) {
        trueObject = view.belief->object;
    }
    if (trueHistory_ != nullptr && !trueHistory_->empty()) {
        trueObject = trueHistory_->back().state.object;
    }
    return trueObject;
}

bool Scorer::committedEffective(int index, const WorldState& pre, Action action,
                                const WorldState& resultState,
                                MoveOutcome resultOutcome,
                                const ScoredView& view) const {
    const FaultEvent& event = events_[static_cast<std::size_t>(index)];
    if (event.type == FaultType::F4) {
        return false;
    }
    if (event.type == FaultType::F3) {
        return committedF3Effective(event, pre, action, resultState,
                                    resultOutcome, view);
    }
    // F1, F2 and F5 build the same shadow state as the
    // observed evaluations, so both use shadowInput (R4).
    const std::optional<WorldState> shadow = shadowInput(
        event, pre, view, trueLayout_, committedTrueObject(pre, view));
    if (!shadow.has_value()) {
        return false;
    }
    if (!isValidWorld(*shadow)) {
        // An invalid shadow counts as effective: the fault
        // is the only reason the true version is impossible.
        return true;
    }
    if (view.model == nullptr) {
        return false;
    }
    const ActionResult shadowResult = view.model->current()(*shadow, action);
    if (shadowResult.outcome != resultOutcome) {
        return true;
    }
    const std::vector<Position> cells = computeFootprintForTwoPredictions(
        pre, resultOutcome, *shadow, shadowResult.outcome, action);
    const FootprintComparison compared =
        compareFootprint(resultState, resultOutcome, shadowResult.state,
                         shadowResult.outcome, cells);
    return compared.outcomesDiffer || !compared.cells.empty();
}

bool Scorer::anyActiveEvent() const {
    for (const EventScore& score : scores_) {
        bool live = score.activated && score.endState == EndState::None;
        live = live && score.active;
        if (live) {
            return true;
        }
    }
    return false;
}

void Scorer::scoreCommittedPrediction(const ScoredView& view) {
    if (view.committed == nullptr || !view.committed->has) {
        return;
    }
    const WorldState& pre = view.committed->beliefBefore;
    const Action action = view.committed->prediction.action;
    const WorldState& resultState = view.committed->prediction.state;
    const MoveOutcome resultOutcome = view.committed->prediction.outcome;
    for (std::size_t i = 0; i < events_.size(); i = i + 1) {
        bool live =
            scores_[i].activated && scores_[i].endState == EndState::None;
        live = live && scores_[i].active;
        if (!live) {
            continue;
        }
        if (!committedEffective(static_cast<int>(i), pre, action, resultState,
                                resultOutcome, view)) {
            continue;
        }
        committedCauses_.push_back(static_cast<int>(i));
        scores_[i].revealing = true;
        if (!scores_[i].opportunityStep.has_value()) {
            scores_[i].opportunityStep = currentStep_;
        }
    }
}

void Scorer::afterDecide(const ScoredView& view, const Decision& decision) {
    // Phase B ends here: everything recorded since the
    // activation point belongs to this phase, after any
    // fault activated at this step.
    endPhase(view);
    if (!decision.isAction) {
        // A safe stop taken while an activated event is
        // still active is justified: stopping was right.
        if (anyActiveEvent()) {
            run_.justifiedSafeStops = run_.justifiedSafeStops + 1;
        }
        return;
    }
    scoreCommittedPrediction(view);
}

}  // namespace aih
