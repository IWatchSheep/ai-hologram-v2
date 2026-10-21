// Purpose: Declares the activation and score lines of a
// fault trace: what happened to every scheduled event,
// written exactly as the golden fault traces write it.
// Introduced in: Stage 4g (rebuild)
// Architecture: The golden fault traces were produced by
// the original C++ and are never edited; the fault trace
// writer must write these lines byte for byte. One line
// is printed whenever its text changes, so an event's
// whole life can be read from the lines that appear.

#ifndef AI_HOLOGRAM_TESTS_FAULT_TRACE_TEXT_H
#define AI_HOLOGRAM_TESTS_FAULT_TRACE_TEXT_H

#include <string>

#include "experiment/fault_injector.h"
#include "experiment/scorer_types.h"

namespace aih {

// One activation record: "activation <id> scheduled",
// "applied at <t>", "not-applicable" or "pending", with
// " | <reason>" appended when the record has a reason.
std::string faultActivationLine(const ActivationRecord& record);

// One event score: whether it activated and is still
// active, how it ended, and the steps at which it gave a
// detection opportunity, a mismatch and a strict detection.
std::string faultScoreLine(const std::string& eventId, const EventScore& score);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_FAULT_TRACE_TEXT_H
