// Purpose: Implements endPhase steps (1) and (2): every new
// correction record is a claim, and every new mismatch
// record is scored against its committed causes (plan 3.7).
// Introduced in: Stage 4d (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. A claim is judged against the events
// active at the START of its phase, never at its end: the
// phase is the time between two scorer calls, and a fault
// activated inside that time cannot have caused a claim
// made at its start. Strict detection needs such a claim
// with the same element as an active event; a claim that
// matches no active event is a false alarm, whatever its
// value, and a claim whose value is not true is a wrong
// accepted correction.

#include "experiment/scorer.h"

#include <cstddef>
#include <vector>

namespace aih {

Element elementOfEvent(const FaultEvent& event) {
    Element element;
    if (event.type == FaultType::F1) {
        element.kind = ElementKind::Wall;
        element.cell = event.cell;
    }
    if (event.type == FaultType::F2 || event.type == FaultType::F5) {
        element.kind = ElementKind::Object;
    }
    if (event.type == FaultType::F3) {
        element.kind = ElementKind::ActionModel;
        element.action = event.action;
    }
    if (event.type == FaultType::F4) {
        element.kind = ElementKind::Trap;
        element.cell = event.cell;
    }
    return element;
}

bool sameElement(const Element& left, const Element& right) {
    if (left.kind != right.kind) {
        return false;
    }
    if (left.kind == ElementKind::Wall || left.kind == ElementKind::Trap) {
        return left.cell == right.cell;
    }
    if (left.kind == ElementKind::ActionModel) {
        return left.action == right.action;
    }
    return true;
}

SourceLabel sourceOf(FaultType type) {
    if (type == FaultType::F3 || type == FaultType::F4) {
        return SourceLabel::S1;
    }
    return SourceLabel::S2;
}

bool Scorer::valueIsTrue(const CorrectionRecord& record,
                         const ScoredView& view) const {
    const std::vector<Observation> noHistory;
    const std::vector<MismatchRecord> noMismatches;
    const std::vector<Observation>& history =
        trueHistory_ != nullptr ? *trueHistory_ : noHistory;
    const std::vector<MismatchRecord>& mismatches =
        view.mismatches != nullptr ? *view.mismatches : noMismatches;
    return claimValueIsTrue(record, trueLayout_, history, mismatches);
}

const CorrectionRecord* Scorer::findRecord(const ScoredView& view,
                                           int id) const {
    if (view.corrections == nullptr) {
        return nullptr;
    }
    for (const CorrectionRecord& record : *view.corrections) {
        if (record.id == id) {
            return &record;
        }
    }
    return nullptr;
}

void Scorer::scoreNewClaims(const ScoredView& view,
                            const std::vector<bool>& activeAtStart,
                            std::vector<const CorrectionRecord*>& newRecords) {
    if (view.corrections == nullptr) {
        return;
    }
    const std::size_t count = events_.size();
    for (std::size_t i = seenCorrections_; i < view.corrections->size();
         i = i + 1) {
        const CorrectionRecord& record = (*view.corrections)[i];
        newRecords.push_back(&record);
        run_.claims = run_.claims + 1;
        if (!valueIsTrue(record, view)) {
            run_.wrongAccepted = run_.wrongAccepted + 1;
        }
        bool found = false;
        for (std::size_t e = 0; e < count; e = e + 1) {
            if (!activeAtStart[e]) {
                continue;
            }
            if (!sameElement(elementOfEvent(events_[e]), record.element)) {
                continue;
            }
            found = true;
            if (!scores_[e].strictStep.has_value()) {
                scores_[e].strictStep = currentStep_;
            }
        }
        if (!found) {
            run_.falseAlarms = run_.falseAlarms + 1;
        }
    }
    seenCorrections_ = view.corrections->size();
}

void Scorer::scoreNewMismatches(const ScoredView& view,
                                const std::vector<bool>& activeAtStart) {
    if (view.mismatches == nullptr) {
        return;
    }
    for (std::size_t i = seenMismatches_; i < view.mismatches->size();
         i = i + 1) {
        const MismatchRecord& record = (*view.mismatches)[i];
        run_.mismatches = run_.mismatches + 1;
        std::vector<int> causes;
        for (const int index : committedCauses_) {
            if (activeAtStart[static_cast<std::size_t>(index)]) {
                causes.push_back(index);
            }
        }
        if (causes.empty()) {
            run_.mismatchesWithoutCause = run_.mismatchesWithoutCause + 1;
            continue;
        }
        run_.labelsScored = run_.labelsScored + 1;
        bool correct = false;
        for (const int index : causes) {
            const FaultEvent& event = events_[static_cast<std::size_t>(index)];
            if (record.suspected == sourceOf(event.type)) {
                correct = true;
            }
            EventScore& score = scores_[static_cast<std::size_t>(index)];
            if (!score.mismatchStep.has_value()) {
                score.mismatchStep = record.decisionStep;
            }
        }
        if (correct) {
            run_.labelsCorrect = run_.labelsCorrect + 1;
        }
    }
    seenMismatches_ = view.mismatches->size();
}

void Scorer::endPhase(const ScoredView& view) {
    const std::size_t count = events_.size();
    std::vector<bool> activeAtStart(count, false);
    for (std::size_t i = 0; i < count; i = i + 1) {
        bool atStart = scores_[i].activated;
        atStart = atStart && scores_[i].endState == EndState::None;
        atStart = atStart && scores_[i].active;
        activeAtStart[i] = atStart;
    }
    std::vector<const CorrectionRecord*> newRecords;
    scoreNewClaims(view, activeAtStart, newRecords);
    scoreNewMismatches(view, activeAtStart);
    updateEventLife(view, newRecords);
    committedCauses_.clear();
}

}  // namespace aih
