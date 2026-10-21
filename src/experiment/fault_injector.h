// Purpose: Declares the activation records and the fault
// injector: what happened to every scheduled event, and
// trying events at the activation point (plan 3.5).
// Introduced in: Stage 4c (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. One injector exists per reasoner per
// run. Amendment A4 in plain words: an F5 fault that
// cannot be applied yet waits (Pending) and is tried again
// at every later step, because the object may move later;
// F1-F4 faults that cannot be applied lapse for good
// (NotApplicable), because their reason will not change
// just by waiting for the object to move.

#ifndef AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_INJECTOR_H
#define AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_INJECTOR_H

#include <string>
#include <vector>

#include "agent/observation.h"
#include "experiment/fault_apply.h"
#include "experiment/fault_event.h"
#include "experiment/fault_model.h"

namespace aih {

// The life of one event for one reasoner (plan 3.5).
enum class ActivationStatus { Scheduled, Applied, NotApplicable, Pending };

// One record per event, in event order. applicationStep is
// when it was Applied; reason is the refusal reason for
// NotApplicable, and the latest refusal reason for Pending.
struct ActivationRecord {
    std::string eventId;
    FaultType type = FaultType::F1;
    int scheduledStep = 0;
    ActivationStatus status = ActivationStatus::Scheduled;
    int applicationStep = 0;
    std::string reason;
};

// applicationStep - scheduledStep when Applied, else 0.
int activationDelay(const ActivationRecord& record);

// Exactly one of the five reporting texts of plan 3.5.
std::string reportedAs(const ActivationRecord& record);

// Applies the scheduled faults of one run to one reasoner.
// The agent never reads the injector; the run with faults
// only calls atActivationPoint through the agent's
// activation hook, and reads the records afterwards.
class FaultInjector {
public:
    // The true history is read (never changed) for F5, and
    // must outlive the injector: the run with faults owns it.
    FaultInjector(std::vector<FaultEvent> events,
                  const std::vector<Observation>* trueHistory);

    // Called once per decision step at the fault activation
    // point: events are gone through in list order, a
    // Scheduled event at this step is tried, and a Pending
    // event is tried at every later step (A4).
    void atActivationPoint(ReasonerParts parts, int decisionStep);

    // One record per event, in event order.
    const std::vector<ActivationRecord>& records() const;

private:
    std::vector<FaultEvent> events_;
    std::vector<ActivationRecord> records_;
    const std::vector<Observation>* trueHistory_;
    FaultedActions faultedActions_;
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_EXPERIMENT_FAULT_INJECTOR_H
