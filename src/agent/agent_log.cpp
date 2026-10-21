// Purpose: Builds the plan, step, safe-stop, V2 settlement
// and result log lines of plan 3.12.
// Introduced in: Stage 3a (rebuild)
// Architecture: These are plain text builders. They make no
// predictions and change nothing; the agent decides when
// each line is written.

#include "agent/agent_log.h"

#include <string>

#include "world/action.h"
#include "world/rules.h"

namespace aih {

std::string planSearchLine(int actions) {
    return "Plan: search found a plan of " + std::to_string(actions) +
           " actions";
}

std::string planFallbackLine(int actions) {
    return "Plan: fallback toward the best state found (" +
           std::to_string(actions) + " actions)";
}

std::string planActiveTestLine(int correctionId, int actions) {
    return "Plan: active test of correction " + std::to_string(correctionId) +
           " (" + std::to_string(actions) + " actions)";
}

std::string planCautiousLine() {
    return "Plan: cautious mode (investigation unfinished)";
}

std::string stepLine(int step, Action action, MoveOutcome predicted,
                     MoveOutcome observed, bool matched) {
    std::string line = "Step " + std::to_string(step) + ": " +
                       actionName(action) + " | predicted " +
                       outcomeName(predicted) + ", observed " +
                       outcomeName(observed);
    if (matched) {
        return line + " | match";
    }
    return line + " | MISMATCH";
}

std::string safeStopLine(int step, const std::string& reason) {
    return "Step " + std::to_string(step) + ": safe stop (" + reason + ")";
}

std::string correctionVerifiedLine(int correctionId) {
    return "  - correction " + std::to_string(correctionId) + ": VERIFIED";
}

std::string correctionRejectedLine(int correctionId) {
    return "  - correction " + std::to_string(correctionId) + ": REJECTED";
}

std::string resultLine(const std::string& outcome, int worldSteps,
                       int safeStops) {
    return "Result: " + outcome + " after " + std::to_string(worldSteps) +
           " world steps, " + std::to_string(safeStops) + " safe stops";
}

std::string mismatchesLine(int mismatches) {
    return "Mismatches: " + std::to_string(mismatches);
}

std::string correctionsLine(int corrections, int verified) {
    return "Corrections: " + std::to_string(corrections) + " (" +
           std::to_string(verified) + " verified)";
}

std::string evaluationsLine(int evaluations) {
    return "Model evaluations used: " + std::to_string(evaluations);
}

}  // namespace aih
