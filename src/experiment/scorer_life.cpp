// Purpose: Implements endPhase step (3): the life of each
// activated fault event (plan 3.7).
// Introduced in: Stage 4d (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. "Active" means the fault is still
// present in the agent. When an active event stops being
// active, it ends in one of two ways: Corrected, when a
// correction record with the true value (or, for F3 and
// F4, a non-rejected record of its element) ended it, so
// the agent gets credit; or Overwritten, when the belief
// became true again with no such record, so it gets none.
// An event Corrected by a record that is later Rejected,
// and active again, becomes active again with no end state.

#include "experiment/scorer.h"

#include <cstddef>
#include <vector>

namespace aih {

void Scorer::endActiveEvent(
    std::size_t index, const ScoredView& view,
    const std::vector<const CorrectionRecord*>& newRecords) {
    const Element element = elementOfEvent(events_[index]);
    const CorrectionRecord* lastTrue = nullptr;
    for (const CorrectionRecord* record : newRecords) {
        if (!sameElement(element, record->element)) {
            continue;
        }
        if (valueIsTrue(*record, view)) {
            lastTrue = record;
        }
    }
    if (lastTrue != nullptr) {
        scores_[index].endState = EndState::Corrected;
        scores_[index].correctingId = lastTrue->id;
    } else if (events_[index].type == FaultType::F3 ||
               events_[index].type == FaultType::F4) {
        int lastId = 0;
        if (view.corrections != nullptr) {
            for (const CorrectionRecord& record : *view.corrections) {
                if (!sameElement(element, record.element)) {
                    continue;
                }
                if (record.status != CorrectionStatus::Rejected) {
                    lastId = record.id;
                }
            }
        }
        scores_[index].endState = EndState::Corrected;
        scores_[index].correctingId = lastId;
    } else {
        scores_[index].endState = EndState::Overwritten;
    }
    scores_[index].endStep = currentStep_;
    scores_[index].active = false;
}

void Scorer::reviveCorrectedEvent(std::size_t index, const ScoredView& view) {
    const CorrectionRecord* record =
        findRecord(view, scores_[index].correctingId);
    if (record == nullptr) {
        return;
    }
    if (record->status != CorrectionStatus::Rejected) {
        return;
    }
    // The correcting record was rejected and the fault is
    // present again: the event is active again.
    scores_[index].endState = EndState::None;
    scores_[index].active = true;
    scores_[index].correctingId = 0;
    scores_[index].endStep = 0;
}

void Scorer::updateEventLife(
    const ScoredView& view,
    const std::vector<const CorrectionRecord*>& newRecords) {
    const std::size_t count = events_.size();
    for (std::size_t i = 0; i < count; i = i + 1) {
        if (!scores_[i].activated) {
            continue;
        }
        const bool now = activeNow(view, static_cast<int>(i));
        bool endsNow = scores_[i].endState == EndState::None;
        endsNow = endsNow && scores_[i].active && !now;
        if (endsNow) {
            endActiveEvent(i, view, newRecords);
        } else if (scores_[i].endState == EndState::Corrected &&
                   scores_[i].correctingId != 0 && now) {
            reviveCorrectedEvent(i, view);
        }
    }
}

}  // namespace aih
