// Purpose: Builds the investigation, correction, consequence
// and anomaly log lines of plan 3.12.
// Introduced in: Stage 3a (rebuild)
// Architecture: These are plain text builders, split from
// agent_log.cpp because the log text has two jobs (plan 4).
// They use the shared positionText and blockedText (rule R4).

#include "agent/agent_log.h"

#include <string>

#include "core/text_words.h"
#include "world/action.h"
#include "world/position.h"

namespace aih {

std::string investigationWallLine(Position cell, bool believedBlocked,
                                  bool recordedBlocked) {
    return "  - investigation: wall at " + positionText(cell) + " believed " +
           blockedText(believedBlocked) + ", recorded " +
           blockedText(recordedBlocked) + " (source S2)";
}

std::string investigationObjectLine(Position believed, Position recorded) {
    return "  - investigation: object believed at " + positionText(believed) +
           ", recorded at " + positionText(recorded) + " (source S2)";
}

std::string investigationModelLine(Action action) {
    return "  - investigation: belief matched the record; action model for " +
           actionName(action) + " suspected (source S1)";
}

std::string anomalyAgentLine() {
    return "  - anomaly: believed agent position differs from the record";
}

std::string investigationDeferredLine() {
    return "  - investigation: budget too small for verification; "
           "retried next step";
}

std::string correctionWallLine(int correctionId, Position cell, bool blocked) {
    return "  - correction " + std::to_string(correctionId) + ": wall at " +
           positionText(cell) + " set to " + blockedText(blocked) +
           ": CONSISTENT";
}

std::string correctionObjectLine(int correctionId, Position position) {
    return "  - correction " + std::to_string(correctionId) +
           ": object position set to " + positionText(position) +
           ": CONSISTENT";
}

std::string correctionModelLine(int correctionId, Action action) {
    return "  - correction " + std::to_string(correctionId) +
           ": action model for " + actionName(action) +
           " restored to the standard rule: CONSISTENT";
}

std::string unexplainedLine() {
    return "  - investigation: no candidate passed V1 (source S5); "
           "unexplained update";
}

std::string consequenceSurpriseLine(Position cell) {
    return "  - consequence surprise: search exhausted with the object at " +
           positionText(cell);
}

std::string fullStateDeadEndLine() {
    return "  - full-state dead end (dead-square classification "
           "consistent; no claim)";
}

std::string consequenceMismatchLine(Position cell) {
    return "  - consequence mismatch: " + positionText(cell) +
           " violates the dead-square rule (source S1)";
}

std::string correctionTrapLine(int correctionId, Position cell) {
    return "  - correction " + std::to_string(correctionId) + ": " +
           positionText(cell) + " classified as a dead square: CONSISTENT";
}

std::string anomalyInvalidLine() {
    return "  - anomaly: invalid hypothetical state skipped";
}

}  // namespace aih
