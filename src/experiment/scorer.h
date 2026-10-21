// Purpose: Declares the scorer (the referee): the views,
// the true-value test for claims, and the Scorer that walks
// each fault event through its life (plan 3.7 and 3.8).
// Introduced in: Stage 4d (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. No agent file includes this header, and
// the scorer never writes to an agent. Each decision step
// has two phases (D-037): phase A ends at the fault
// activation point, before any fault is applied, and phase
// B ends when decide returns. A claim is therefore judged
// against the events active at the START of its phase,
// because a claim made in phase A was made before a fault
// activated at that step could have caused it.

#ifndef AI_HOLOGRAM_SRC_EXPERIMENT_SCORER_H
#define AI_HOLOGRAM_SRC_EXPERIMENT_SCORER_H

#include <cstddef>
#include <vector>

#include "agent/agent_types.h"
#include "agent/naive_agent.h"
#include "agent/observation.h"
#include "agent/records.h"
#include "agent/single_agent_state.h"
#include "experiment/fault_event.h"
#include "experiment/fault_injector.h"
#include "experiment/scorer_types.h"
#include "world/action.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {

// The read-only view of one C1 or C0 state (the last three
// pointers are null for C0).
ScoredView viewOf(const SingleAgentState& state);
ScoredView viewOf(const NaiveAgentState& state);

// Whether the value claimed by one correction record is the
// true value (plan 3.8). An object claim is judged at the
// moment it is about (D-042): the true object in
// trueHistory[s - 2], where s is the latest mismatch
// record's step; it is false when there is no mismatch,
// s < 2, or that index is out of range.
bool claimValueIsTrue(const CorrectionRecord& record,
                      const WorldState& trueLayout,
                      const std::vector<Observation>& trueHistory,
                      const std::vector<MismatchRecord>& mismatches);

// The element one fault event is about (plan 3.7). This is
// the one copy, shared by the claims and the event life.
Element elementOfEvent(const FaultEvent& event);

// Two elements are the same when their kinds match and,
// for wall and trap, their cells match, or, for the action
// model, their actions match. The object has no key.
bool sameElement(const Element& left, const Element& right);

// The true source of one fault type (plan 3.8): faults in
// the model or the trap analysis are S1, belief faults S2.
SourceLabel sourceOf(FaultType type);

// Scores one run from the agent's own records. Credit comes
// only through records: a belief that becomes true without
// a correction record is Overwritten, never a correction.
class Scorer {
public:
    // The true history is read (never changed) and must
    // outlive the scorer: the run with faults owns it.
    Scorer(std::vector<FaultEvent> events, WorldState trueLayout,
           const std::vector<Observation>* trueHistory);

    // Ends phase A of one step, before any fault is applied.
    void endPhaseA(const ScoredView& view, int decisionStep);
    // Records which events the injector has just applied.
    void afterActivation(const ScoredView& view,
                         const std::vector<ActivationRecord>& records,
                         int decisionStep);
    // Ends phase B of one step, then the committed shadow.
    void afterDecide(const ScoredView& view, const Decision& decision);
    // Ends the final-check phase, dated to the last step.
    void endFinalCheck(const ScoredView& view);
    // Records each Corrected event's final record status.
    void finish(const ScoredView& view);

    const std::vector<EventScore>& eventScores() const;
    const RunScore& runScore() const;

private:
    // Whether the event's fault is present in the agent now.
    bool activeNow(const ScoredView& view, int index) const;
    // claimValueIsTrue with this scorer's layout and history.
    bool valueIsTrue(const CorrectionRecord& record,
                     const ScoredView& view) const;
    const CorrectionRecord* findRecord(const ScoredView& view, int id) const;
    // One phase, in the order of plan 3.7: (1) claims,
    // (2) mismatch records, (3) the event life; then the
    // committed causes are cleared.
    void endPhase(const ScoredView& view);
    void scoreNewClaims(const ScoredView& view,
                        const std::vector<bool>& activeAtStart,
                        std::vector<const CorrectionRecord*>& newRecords);
    void scoreNewMismatches(const ScoredView& view,
                            const std::vector<bool>& activeAtStart);
    void updateEventLife(
        const ScoredView& view,
        const std::vector<const CorrectionRecord*>& newRecords);
    // Ends one event that has just stopped being active.
    void endActiveEvent(std::size_t index, const ScoredView& view,
                        const std::vector<const CorrectionRecord*>& newRecords);
    // Revives one Corrected event whose record was rejected.
    void reviveCorrectedEvent(std::size_t index, const ScoredView& view);
    // The committed-prediction shadow of plan 3.9.
    bool committedEffective(int index, const WorldState& pre, Action action,
                            const WorldState& resultState,
                            MoveOutcome resultOutcome,
                            const ScoredView& view) const;
    bool committedF3Effective(const FaultEvent& event, const WorldState& pre,
                              Action action, const WorldState& resultState,
                              MoveOutcome resultOutcome,
                              const ScoredView& view) const;
    // Fills shadow with pre corrected to the true value;
    // false means this event has no shadow here.
    bool makeCommittedShadow(const FaultEvent& event, const WorldState& pre,
                             const ScoredView& view, WorldState& shadow) const;
    bool anyActiveEvent() const;
    void scoreCommittedPrediction(const ScoredView& view);

    std::vector<FaultEvent> events_;
    std::vector<EventScore> scores_;
    WorldState trueLayout_;
    const std::vector<Observation>* trueHistory_;
    RunScore run_;
    int currentStep_ = 0;
    std::size_t seenCorrections_ = 0;
    std::size_t seenMismatches_ = 0;
    std::vector<int> committedCauses_;
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_EXPERIMENT_SCORER_H
