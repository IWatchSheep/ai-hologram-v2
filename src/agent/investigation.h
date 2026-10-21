// Purpose: Declares the investigation of a mismatch and
// the consequence check (plan 3.7).
// Introduced in: Stage 3d (rebuild)
// Architecture: investigateMismatch lives in
// investigation.cpp and checkConsequence in consequence.cpp
// (the planned split, plan Section 4). Both only read the
// belief and the history; applying what they find is the
// agent's job in 3e.

#ifndef AI_HOLOGRAM_SRC_AGENT_INVESTIGATION_H
#define AI_HOLOGRAM_SRC_AGENT_INVESTIGATION_H

#include <vector>

#include "agent/dead_squares.h"
#include "agent/observation.h"
#include "agent/records.h"
#include "agent/step_predictor.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// One belief candidate: the element it is about, and the
// recorded value (blocked for a wall, position for the
// object).
struct BeliefCandidate {
    Element element{};
    bool blocked = false;
    Position position{};
};

// How an investigation ended: the candidates agree with the
// record, no candidate passed, the action model is suspected,
// or the budget was too small and the work is deferred.
enum class InvestigationOutcome {
    BeliefCandidatesConsistent,
    NoCandidatePassed,
    ModelSuspected,
    Deferred
};

// The result of one investigation, with the defaults of the
// reference: the model is suspected, source S5, no anomaly.
struct InvestigationResult {
    InvestigationOutcome outcome = InvestigationOutcome::ModelSuspected;
    std::vector<BeliefCandidate> candidates;
    SourceLabel suspected = SourceLabel::S5;
    bool anomaly = false;
};

// Investigates the mismatch of action, in exactly the order
// of plan 3.7: the anomaly flag, the candidates from the
// footprint, the history checks and the corrected belief's
// validity, then the reserve test, then one variant
// prediction with the current model, compared on the
// footprint. A variant prediction is used here because the
// corrected belief is hypothetical: its prediction must not
// be cached as the agent's own prediction.
InvestigationResult investigateMismatch(const WorldState& beliefBefore,
                                        Action action,
                                        const std::vector<Observation>& history,
                                        StepPredictor& predictor,
                                        const ActionModel& currentModel,
                                        int reserve);

// How a consequence check ended (plan 3.7).
enum class ConsequenceOutcome { NotApplicable, FullStateDeadEnd, Mismatch };

// The consequence check: whether the believed object cell p
// being unsolvable is a full-state dead end (no claim) or a
// mismatch with the dead-square rule. It reads p's dead
// status only through readDeadStatus (IE-1 H4).
ConsequenceOutcome checkConsequence(const WorldState& belief,
                                    const DeadSquareMap& map,
                                    const std::vector<Observation>& history,
                                    FreeComputationCounts& counts);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_INVESTIGATION_H
