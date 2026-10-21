// Purpose: Implements the activation records and the
// FaultInjector (plan 3.5): trying each event at its step,
// and retrying a refused F5 at every later step (A4).
// Introduced in: Stage 4c (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. The per-type application itself lives
// in fault_apply.cpp. No agent ever calls this code; the
// run with faults calls it through the activation hook.

#include "experiment/fault_injector.h"

#include <cstddef>
#include <utility>
#include <vector>

#include "agent/observation.h"
#include "experiment/fault_apply.h"
#include "experiment/fault_event.h"

namespace aih {

int activationDelay(const ActivationRecord& record) {
    if (record.status != ActivationStatus::Applied) {
        return 0;
    }
    return record.applicationStep - record.scheduledStep;
}

std::string reportedAs(const ActivationRecord& record) {
    if (record.status == ActivationStatus::Applied) {
        if (record.applicationStep == record.scheduledStep) {
            return "Activated at its scheduled step";
        }
        return "Activated after deferral";
    }
    if (record.status == ActivationStatus::Scheduled) {
        return "Not activated: run ended before the scheduled step";
    }
    if (record.status == ActivationStatus::NotApplicable) {
        return "Not activated: not applicable at the scheduled step";
    }
    return "Not activated: deferred, never applicable";
}

FaultInjector::FaultInjector(std::vector<FaultEvent> events,
                             const std::vector<Observation>* trueHistory)
    : events_(std::move(events)), trueHistory_(trueHistory) {
    for (const FaultEvent& event : events_) {
        ActivationRecord record;
        record.eventId = event.id;
        record.type = event.type;
        record.scheduledStep = event.step;
        records_.push_back(record);
    }
}

const std::vector<ActivationRecord>& FaultInjector::records() const {
    return records_;
}

void FaultInjector::atActivationPoint(ReasonerParts parts, int decisionStep) {
    // Events are gone through in list order, every time.
    for (std::size_t i = 0; i < events_.size(); i = i + 1) {
        ActivationRecord& record = records_[i];
        bool shouldTry = false;
        if (record.status == ActivationStatus::Scheduled) {
            if (record.scheduledStep == decisionStep) {
                shouldTry = true;
            }
        }
        if (record.status == ActivationStatus::Pending) {
            if (decisionStep > record.scheduledStep) {
                shouldTry = true;
            }
        }
        if (!shouldTry) {
            continue;
        }
        const std::vector<Observation> emptyHistory;
        const std::vector<Observation>& history =
            trueHistory_ != nullptr ? *trueHistory_ : emptyHistory;
        const ApplyOutcome outcome =
            tryApplyFault(events_[i], parts, faultedActions_, history);
        if (outcome.applied) {
            record.status = ActivationStatus::Applied;
            record.applicationStep = decisionStep;
            record.reason = "";
        } else {
            record.reason = outcome.reason;
            if (events_[i].type == FaultType::F5) {
                record.status = ActivationStatus::Pending;
            } else {
                record.status = ActivationStatus::NotApplicable;
            }
        }
    }
}

}  // namespace aih
