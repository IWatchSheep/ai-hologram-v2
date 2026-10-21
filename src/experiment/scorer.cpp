// Purpose: Implements the scorer's read-only views, the
// true-value test for claims, "active now", the phase entry
// points and finish (plan 3.7 and 3.8).
// Introduced in: Stage 4d (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. The scorer only reads an agent through
// the view's const pointers; it never writes to it, because
// a referee that helped the agent would spoil the measure.
// An event is active while its fault is still present in
// the agent: a wrong wall, a wrong object, an unrestored
// action, or a "not dead" cell not yet corrected to dead.

#include "experiment/scorer.h"

#include <cstddef>
#include <utility>
#include <vector>

#include "agent/dead_squares.h"
#include "world/position.h"

namespace aih {

ScoredView viewOf(const SingleAgentState& state) {
    ScoredView view;
    view.belief = &state.belief;
    view.model = &state.agentModel;
    view.notDeadOverrides = &state.notDeadOverrides;
    view.deadOverrides = &state.deadOverrides;
    view.corrections = &state.corrections;
    view.mismatches = &state.mismatches;
    view.committed = &state.committed;
    return view;
}

ScoredView viewOf(const NaiveAgentState& state) {
    ScoredView view;
    view.belief = &state.belief;
    view.model = &state.agentModel;
    view.notDeadOverrides = &state.notDeadOverrides;
    view.deadOverrides = &state.deadOverrides;
    return view;
}

bool claimValueIsTrue(const CorrectionRecord& record,
                      const WorldState& trueLayout,
                      const std::vector<Observation>& trueHistory,
                      const std::vector<MismatchRecord>& mismatches) {
    if (record.element.kind == ElementKind::Wall) {
        return record.newBlocked == isWall(trueLayout, record.element.cell);
    }
    if (record.element.kind == ElementKind::ActionModel) {
        // The only model correction is "the standard rule",
        // which is the true behaviour, so it is always true.
        return true;
    }
    if (record.element.kind == ElementKind::Trap) {
        FreeComputationCounts counts;
        const DeadSquareMap map =
            computeDeadSquares(trueLayout, {}, {}, counts);
        if (!isInsideGrid(record.element.cell)) {
            return false;
        }
        const std::size_t row = static_cast<std::size_t>(record.element.cell.y);
        const std::size_t column =
            static_cast<std::size_t>(record.element.cell.x);
        return map.dead[row][column];
    }
    // Object (D-042): judged against the true object just
    // before the investigated action (see scorer.h).
    if (mismatches.empty()) {
        return false;
    }
    const int step = mismatches.back().decisionStep;
    if (step < 2) {
        return false;
    }
    const std::size_t index = static_cast<std::size_t>(step - 2);
    if (index >= trueHistory.size()) {
        return false;
    }
    return record.newPosition == trueHistory[index].state.object;
}

Scorer::Scorer(std::vector<FaultEvent> events, WorldState trueLayout,
               const std::vector<Observation>* trueHistory)
    : events_(std::move(events)),
      trueLayout_(trueLayout),
      trueHistory_(trueHistory) {
    scores_.resize(events_.size());
}

const std::vector<EventScore>& Scorer::eventScores() const {
    return scores_;
}

const RunScore& Scorer::runScore() const {
    return run_;
}

bool Scorer::activeNow(const ScoredView& view, int index) const {
    const FaultEvent& event = events_[static_cast<std::size_t>(index)];
    if (event.type == FaultType::F1) {
        if (view.belief == nullptr) {
            return false;
        }
        return isWall(*view.belief, event.cell) !=
               isWall(trueLayout_, event.cell);
    }
    if (event.type == FaultType::F2 || event.type == FaultType::F5) {
        if (view.belief == nullptr) {
            return false;
        }
        if (trueHistory_ == nullptr || trueHistory_->empty()) {
            return false;
        }
        return view.belief->object != trueHistory_->back().state.object;
    }
    if (event.type == FaultType::F3) {
        if (view.model == nullptr) {
            return false;
        }
        return !view.model->isRestored(event.action);
    }
    // F4 stays active until the cell is a "dead" correction.
    if (view.deadOverrides == nullptr) {
        return true;
    }
    for (const Position cell : *view.deadOverrides) {
        if (cell == event.cell) {
            return false;
        }
    }
    return true;
}

void Scorer::endPhaseA(const ScoredView& view, int decisionStep) {
    currentStep_ = decisionStep;
    endPhase(view);
}

void Scorer::afterActivation(const ScoredView& view,
                             const std::vector<ActivationRecord>& records,
                             int decisionStep) {
    currentStep_ = decisionStep;
    for (std::size_t i = 0; i < events_.size(); i = i + 1) {
        if (i >= records.size()) {
            continue;
        }
        if (records[i].status != ActivationStatus::Applied) {
            continue;
        }
        if (scores_[i].activated) {
            continue;
        }
        scores_[i].activated = true;
        scores_[i].activationStep = decisionStep;
        scores_[i].active = true;
        if (!activeNow(view, static_cast<int>(i))) {
            // The fault ended at once (for example F2 moved
            // the believed object onto the true one), with
            // no record: Overwritten at this step.
            scores_[i].active = false;
            scores_[i].endState = EndState::Overwritten;
            scores_[i].endStep = decisionStep;
        }
    }
}

void Scorer::endFinalCheck(const ScoredView& view) {
    // D-045: the final check is dated to the last decision
    // step, so the current step is left unchanged.
    endPhase(view);
}

void Scorer::finish(const ScoredView& view) {
    if (view.corrections == nullptr) {
        return;
    }
    for (EventScore& score : scores_) {
        if (score.endState != EndState::Corrected) {
            continue;
        }
        if (score.correctingId == 0) {
            continue;
        }
        for (const CorrectionRecord& record : *view.corrections) {
            if (record.id == score.correctingId) {
                score.correctionStatus = record.status;
            }
        }
    }
}

}  // namespace aih
