// Purpose: Declares the read-only view of one reasoner and
// the scores the scorer keeps for each event and for the
// whole run (plan 3.7 and 3.10).
// Introduced in: Stage 4d (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. The scorer only reads an agent through
// the pointers of ScoredView; it never writes to an agent,
// because it measures the agent and must never help it.
// For C0 the last three pointers are null: C0 keeps no
// corrections, no mismatches and no committed prediction,
// so its events can only end Overwritten or stay active
// (quirk Q10).

#ifndef AI_HOLOGRAM_SRC_EXPERIMENT_SCORER_TYPES_H
#define AI_HOLOGRAM_SRC_EXPERIMENT_SCORER_TYPES_H

#include <optional>
#include <vector>

#include "agent/agent_model.h"
#include "agent/records.h"
#include "agent/single_agent_state.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// A read-only view of one reasoner. Every pointer is to
// const data, so code that holds a view cannot change the
// agent through it.
struct ScoredView {
    const WorldState* belief = nullptr;
    const AgentActionModel* model = nullptr;
    const std::vector<Position>* notDeadOverrides = nullptr;
    const std::vector<Position>* deadOverrides = nullptr;
    const std::vector<CorrectionRecord>* corrections = nullptr;
    const std::vector<MismatchRecord>* mismatches = nullptr;
    const CommittedPrediction* committed = nullptr;
};

// How one activated event ended. None with activated true
// means the event is still active at the end of the run;
// None with activated false means it was never activated.
// Corrected means a correction record ended it; Overwritten
// means the belief became true again without such a record,
// so no credit is given (plan 3.7).
enum class EndState { None, Corrected, Overwritten };

// The scored life of one fault event.
struct EventScore {
    bool activated = false;
    int activationStep = 0;
    bool active = false;
    EndState endState = EndState::None;
    int endStep = 0;
    int correctingId = 0;
    std::optional<CorrectionStatus> correctionStatus;
    std::optional<int> opportunityStep;
    bool revealing = false;
    std::optional<int> mismatchStep;
    std::optional<int> strictStep;
};

// The scored facts of one whole run.
struct RunScore {
    int claims = 0;
    int falseAlarms = 0;
    int wrongAccepted = 0;
    int justifiedSafeStops = 0;
    int mismatches = 0;
    int mismatchesWithoutCause = 0;
    int labelsCorrect = 0;
    int labelsScored = 0;
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_EXPERIMENT_SCORER_TYPES_H
