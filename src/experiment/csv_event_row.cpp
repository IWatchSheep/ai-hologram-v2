// Purpose: Implements the events.csv row (plan 3.11).
// Introduced in: Stage 4f (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. An event that was not activated has
// empty measurement fields, not zeros: empty means "did
// not happen", while 0 would look like a measured miss
// (D-044).

#include "experiment/csv_output.h"

#include <string>
#include <vector>

#include "agent/records.h"
#include "experiment/csv_fields.h"
#include "experiment/fault_event.h"
#include "experiment/fault_injector.h"
#include "experiment/scorer_types.h"
#include "world/action.h"

namespace aih {
namespace {

// The five detail fields of one event: cell x:y and the
// F1 direction for F1, cell for F4, the direction order
// with single spaces for F2, action and rotation for F3.
void addEventDetails(std::vector<std::string>& fields,
                     const FaultEvent& event) {
    std::string cell;
    if (event.type == FaultType::F1 || event.type == FaultType::F4) {
        if (event.hasCell) {
            cell = std::to_string(event.cell.x) + ":" +
                   std::to_string(event.cell.y);
        }
    }
    fields.push_back(cell);
    std::string direction;
    if (event.type == FaultType::F1) {
        direction = f1DirectionName(event.direction);
    }
    fields.push_back(direction);
    std::string order;
    if (event.type == FaultType::F2) {
        order = actionName(event.order[0]) + " " + actionName(event.order[1]) +
                " " + actionName(event.order[2]) + " " +
                actionName(event.order[3]);
    }
    fields.push_back(order);
    std::string action;
    std::string rotation;
    if (event.type == FaultType::F3) {
        action = actionName(event.action);
        rotation = rotationName(event.rotation);
    }
    fields.push_back(action);
    fields.push_back(rotation);
}

// The measurement fields of an event never activated:
// activation step, delay and every measurement are
// empty; only the refusal reason may be present (D-044).
void addNotActivatedFields(std::vector<std::string>& fields,
                           const ActivationRecord& activation) {
    fields.push_back("");
    fields.push_back("");
    std::string reason;
    if (activation.status == ActivationStatus::NotApplicable) {
        reason = activation.reason;
    }
    if (activation.status == ActivationStatus::Pending) {
        reason = activation.reason;
    }
    fields.push_back(reason);
    for (int i = 0; i < 12; i = i + 1) {
        fields.push_back("");
    }
}

// How one activated event ended, in the CSV's words.
std::string endStateText(EndState endState) {
    if (endState == EndState::Corrected) {
        return "Corrected";
    }
    if (endState == EndState::Overwritten) {
        return "Overwritten without correction";
    }
    return "Active at end of run";
}

// The correction status word, only for a Corrected event
// whose correcting record has a final status.
std::string correctionStatusText(const EventScore& score) {
    if (score.endState != EndState::Corrected) {
        return "";
    }
    if (!score.correctionStatus.has_value()) {
        return "";
    }
    if (score.correctionStatus.value() == CorrectionStatus::Consistent) {
        return "CONSISTENT";
    }
    if (score.correctionStatus.value() == CorrectionStatus::Verified) {
        return "VERIFIED";
    }
    return "REJECTED";
}

// The measurement fields of an activated event. The
// detection delay counts from the activation
// (application) step, not from the scheduled step: a
// deferred fault could not be detected before it existed.
void addActivatedFields(std::vector<std::string>& fields,
                        const ActivationRecord& activation,
                        const EventScore& score) {
    fields.push_back(std::to_string(activation.applicationStep));
    fields.push_back(std::to_string(activationDelay(activation)));
    fields.push_back("");
    fields.push_back(bitText(score.opportunityStep.has_value()));
    fields.push_back(optionalStepText(score.opportunityStep));
    fields.push_back(bitText(score.revealing));
    fields.push_back(bitText(score.mismatchStep.has_value()));
    fields.push_back(optionalStepText(score.mismatchStep));
    fields.push_back(bitText(score.strictStep.has_value()));
    fields.push_back(optionalStepText(score.strictStep));
    std::string delay;
    if (score.strictStep.has_value()) {
        delay = std::to_string(score.strictStep.value() -
                               activation.applicationStep);
    }
    fields.push_back(delay);
    fields.push_back(endStateText(score.endState));
    std::string endStep;
    if (score.endState != EndState::None) {
        endStep = std::to_string(score.endStep);
    }
    fields.push_back(endStep);
    fields.push_back(correctionStatusText(score));
    bool verified = false;
    if (score.correctionStatus.has_value()) {
        verified = score.correctionStatus.value() == CorrectionStatus::Verified;
    }
    fields.push_back(bitText(verified));
}

}  // namespace

std::string eventCsvRow(const std::string& condition, const FaultEvent& event,
                        const ActivationRecord& activation,
                        const EventScore& score) {
    std::vector<std::string> fields;
    fields.push_back(event.id);
    fields.push_back(condition);
    fields.push_back("R");
    fields.push_back(std::to_string(event.layoutSeed));
    fields.push_back(std::to_string(event.runSeed));
    fields.push_back(faultTypeName(event.type));
    fields.push_back(faultTargetName(event.target));
    fields.push_back(bitText(event.commonMode));
    addEventDetails(fields, event);
    fields.push_back(std::to_string(event.step));
    // Quirk Q7 (D-078): the reported_as text "Not activated:
    // deferred, never applicable" contains a comma and is
    // written as it is, unquoted, so that row has 31
    // comma-separated pieces instead of 30. This is the
    // original's output: it is kept, never repaired.
    fields.push_back(reportedAs(activation));
    if (activation.status != ActivationStatus::Applied) {
        addNotActivatedFields(fields, activation);
    } else {
        addActivatedFields(fields, activation, score);
    }
    return joinFields(fields);
}

}  // namespace aih
