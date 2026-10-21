// Purpose: Implements the activation and score lines of
// a fault trace, following ref_fault_trace.py exactly.
// Introduced in: Stage 4g (rebuild)
// Architecture: The words of these lines are the golden
// files' words. Yes and no are spelled as the Stage 3
// trace text spells them, and a step that does not exist
// is written as a dash, never as zero, so "did not
// happen" can never look like a measured step.

#include "fault_trace_text.h"

#include <optional>
#include <string>

#include "experiment/fault_injector.h"
#include "experiment/scorer_types.h"

namespace aih {
namespace {

// "yes" or "no", as the Stage 3 trace text spells them.
std::string yesNo(bool value) {
    if (value) {
        return "yes";
    }
    return "no";
}

// A step as text, or "-" when the step does not exist.
std::string stepText(const std::optional<int>& step) {
    if (!step.has_value()) {
        return "-";
    }
    return std::to_string(step.value());
}

// The status word of an activation record.
std::string statusText(ActivationStatus status) {
    if (status == ActivationStatus::Applied) {
        return "applied";
    }
    if (status == ActivationStatus::NotApplicable) {
        return "not-applicable";
    }
    if (status == ActivationStatus::Pending) {
        return "pending";
    }
    return "scheduled";
}

// How one activated event ended, in the trace's words.
std::string endText(EndState endState) {
    if (endState == EndState::Corrected) {
        return "corrected";
    }
    if (endState == EndState::Overwritten) {
        return "overwritten";
    }
    return "none";
}

}  // namespace

std::string faultActivationLine(const ActivationRecord& record) {
    std::string line =
        "activation " + record.eventId + " " + statusText(record.status);
    if (record.status == ActivationStatus::Applied) {
        line += " at " + std::to_string(record.applicationStep);
    }
    if (!record.reason.empty()) {
        line += " | " + record.reason;
    }
    return line;
}

std::string faultScoreLine(const std::string& eventId,
                           const EventScore& score) {
    std::string line =
        "score " + eventId + " activated " + yesNo(score.activated) + " at " +
        std::to_string(score.activationStep) + " active " +
        yesNo(score.active) + " end " + endText(score.endState) + " " +
        std::to_string(score.endStep) + " by " +
        std::to_string(score.correctingId) + " opportunity " +
        stepText(score.opportunityStep) + " revealing " +
        yesNo(score.revealing) + " mismatch " + stepText(score.mismatchStep) +
        " strict " + stepText(score.strictStep);
    return line;
}

}  // namespace aih
