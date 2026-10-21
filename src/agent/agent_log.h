// Purpose: Declares every log-line builder of plan 3.12,
// with exactly that text.
// Introduced in: Stage 3a (rebuild)
// Architecture: The agent and the run loop only append lines
// made here, so the exact wording lives in one place. The
// lines use positionText, blockedText, actionName and
// outcomeName; no copy of those helpers is written (rule R4).

#ifndef AI_HOLOGRAM_SRC_AGENT_AGENT_LOG_H
#define AI_HOLOGRAM_SRC_AGENT_AGENT_LOG_H

#include <string>

#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"

namespace aih {

// The plan lines.
std::string planSearchLine(int actions);
std::string planFallbackLine(int actions);
std::string planActiveTestLine(int correctionId, int actions);
std::string planCautiousLine();

// The step, safe-stop and V2 settlement lines.
std::string stepLine(int step, Action action, MoveOutcome predicted,
                     MoveOutcome observed, bool matched);
std::string safeStopLine(int step, const std::string& reason);
std::string correctionVerifiedLine(int correctionId);
std::string correctionRejectedLine(int correctionId);

// The investigation lines.
std::string investigationWallLine(Position cell, bool believedBlocked,
                                  bool recordedBlocked);
std::string investigationObjectLine(Position believed, Position recorded);
std::string investigationModelLine(Action action);
std::string anomalyAgentLine();
std::string investigationDeferredLine();

// The correction lines.
std::string correctionWallLine(int correctionId, Position cell, bool blocked);
std::string correctionObjectLine(int correctionId, Position position);
std::string correctionModelLine(int correctionId, Action action);
std::string unexplainedLine();

// The consequence and anomaly lines.
std::string consequenceSurpriseLine(Position cell);
std::string fullStateDeadEndLine();
std::string consequenceMismatchLine(Position cell);
std::string correctionTrapLine(int correctionId, Position cell);
std::string anomalyInvalidLine();

// The result lines at the end of a run.
std::string resultLine(const std::string& outcome, int worldSteps,
                       int safeStops);
std::string mismatchesLine(int mismatches);
std::string correctionsLine(int corrections, int verified);
std::string evaluationsLine(int evaluations);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_AGENT_LOG_H
