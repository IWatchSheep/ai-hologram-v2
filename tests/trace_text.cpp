// Purpose: Implements the exact trace text of the gate,
// following ref_trace.py line by line (plan 3.14).
// Introduced in: Stage 3g (rebuild)
// Architecture: The golden traces were written in this
// text from the original code, so every word, space and
// number here must match it. The shared text helpers
// positionText and blockedText write positions and wall
// values (rule R4); action and outcome names are the
// world's own.

#include "trace_text.h"

#include <string>
#include <vector>

#include "core/text_words.h"
#include "world/action.h"
#include "world/rules.h"

namespace aih {
namespace {

// "yes" or "no", the trace's words for a boolean.
std::string yesNo(bool value) {
    return value ? "yes" : "no";
}

// "1" or "0", for one value of a footprint cell difference.
std::string bitText(bool value) {
    return value ? "1" : "0";
}

// The source label of a mismatch, S1 to S5.
std::string sourceText(SourceLabel source) {
    if (source == SourceLabel::S1) {
        return "S1";
    }
    if (source == SourceLabel::S2) {
        return "S2";
    }
    if (source == SourceLabel::S3) {
        return "S3";
    }
    if (source == SourceLabel::S4) {
        return "S4";
    }
    return "S5";
}

// The status word of a correction record.
std::string statusText(CorrectionStatus status) {
    if (status == CorrectionStatus::Verified) {
        return "VERIFIED";
    }
    if (status == CorrectionStatus::Rejected) {
        return "REJECTED";
    }
    return "CONSISTENT";
}

}  // namespace

std::string hexWalls(const WorldState& state) {
    const char digits[] = "0123456789abcdef";
    std::string text;
    for (int j = 0; j < 25; j = j + 1) {
        int value = 0;
        for (int k = 0; k < 4; k = k + 1) {
            const int cell = 4 * j + k;
            const std::size_t x = static_cast<std::size_t>(cell % 10);
            const std::size_t y = static_cast<std::size_t>(cell / 10);
            value = value * 2 + (state.walls[y][x] ? 1 : 0);
        }
        text.push_back(digits[value]);
    }
    return text;
}

std::string stateText(const WorldState& state) {
    return "agent " + positionText(state.agent) + " object " +
           positionText(state.object) + " destination " +
           positionText(state.destination) + " walls " + hexWalls(state);
}

std::string elementText(const Element& element) {
    if (element.kind == ElementKind::ActionModel) {
        return "model " + actionName(element.action);
    }
    if (element.kind == ElementKind::Object) {
        return "object";
    }
    if (element.kind == ElementKind::Trap) {
        return "trap " + positionText(element.cell);
    }
    return "wall " + positionText(element.cell);
}

std::string observationLine(const Observation& observation) {
    std::string line = "observation " +
                       std::to_string(observation.decisionStep) + " " +
                       stateText(observation.state);
    if (observation.lastAction.has_value()) {
        const MoveOutcome outcome = observation.lastOutcome.has_value()
                                        ? observation.lastOutcome.value()
                                        : MoveOutcome::Blocked;
        line += " last " + actionName(observation.lastAction.value()) + " " +
                outcomeName(outcome);
    } else {
        line += " last none";
    }
    return line;
}

std::vector<std::string> mismatchLines(const MismatchRecord& mismatch) {
    std::vector<std::string> lines;
    lines.push_back("mismatch step " + std::to_string(mismatch.decisionStep) +
                    " action " + actionName(mismatch.action) + " outcomes " +
                    (mismatch.differences.outcomesDiffer ? "differ" : "same") +
                    " suspected " + sourceText(mismatch.suspected) +
                    " anomaly " + yesNo(mismatch.anomaly));
    for (const FootprintCellDifference& cell : mismatch.differences.cells) {
        lines.push_back("  cell " + positionText(cell.cell) + " wall " +
                        bitText(cell.predictedWall) +
                        bitText(cell.observedWall) + " agent " +
                        bitText(cell.predictedAgentHere) +
                        bitText(cell.observedAgentHere) + " object " +
                        bitText(cell.predictedObjectHere) +
                        bitText(cell.observedObjectHere));
    }
    for (const Element& candidate : mismatch.candidates) {
        lines.push_back("  candidate " + elementText(candidate));
    }
    return lines;
}

std::string correctionLine(const CorrectionRecord& record) {
    std::string line = "correction " + std::to_string(record.id) + " " +
                       elementText(record.element);
    if (record.element.kind == ElementKind::Wall) {
        line += " new " + blockedText(record.newBlocked) + " old " +
                blockedText(record.oldBlocked);
    }
    if (record.element.kind == ElementKind::Object) {
        line += " new " + positionText(record.newPosition) + " old " +
                positionText(record.oldPosition);
    }
    return line + " " + statusText(record.status) + " since " +
           std::to_string(record.stepConsistent) + " tested " +
           yesNo(record.tested);
}

std::string traceResultLine(const std::string& outcome, int worldSteps,
                            int decisionSteps, int safeStops, int mismatches,
                            int corrections, int verified, int evaluations) {
    return "result " + outcome + " | world steps " +
           std::to_string(worldSteps) + " | decision steps " +
           std::to_string(decisionSteps) + " | safe stops " +
           std::to_string(safeStops) + " | mismatches " +
           std::to_string(mismatches) + " | corrections " +
           std::to_string(corrections) + " | verified " +
           std::to_string(verified) + " | evaluations " +
           std::to_string(evaluations);
}

}  // namespace aih
