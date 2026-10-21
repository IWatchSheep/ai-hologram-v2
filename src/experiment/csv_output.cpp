// Purpose: Implements the two CSV headers, the runs.csv
// row, and the file-writing helper (plan 3.11).
// Introduced in: Stage 4f (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. The event row lives in
// csv_event_row.cpp. Rows are made only from records;
// this file reads nothing from an agent directly.

#include "experiment/csv_output.h"

#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include "experiment/csv_fields.h"
#include "experiment/fault_injector.h"
#include "experiment/fault_run.h"

namespace aih {

std::string runCsvHeader() {
    return "run_id,condition,layout_seed,run_seed,outcome,decision_steps,"
           "world_steps,safe_stops,justified_safe_stops,mismatches,"
           "mismatches_without_fault_cause,source_labels_correct,"
           "source_labels_scored,claims,false_alarms,"
           "wrong_accepted_corrections,evaluations,events_scheduled,"
           "events_activated";
}

std::string eventCsvHeader() {
    return "event_id,condition,reasoner,layout_seed,run_seed,type,target,"
           "common_mode,cell,direction,order,action,rotation,scheduled_step,"
           "reported_as,activation_step,activation_delay,"
           "not_activated_reason,opportunity,opportunity_step,"
           "revealing_opportunity,mismatch_detected,mismatch_step,"
           "strictly_detected,strict_step,detection_delay,end_state,"
           "end_step,correction_status,verified_correction";
}

std::string runCsvRow(const std::string& condition, std::uint64_t layoutSeed,
                      int runSeed, const FaultRunResult& result) {
    int activated = 0;
    for (const ActivationRecord& record : result.activations) {
        if (record.status == ActivationStatus::Applied) {
            activated = activated + 1;
        }
    }
    std::vector<std::string> fields;
    fields.push_back(std::to_string(layoutSeed) + "-" +
                     std::to_string(runSeed));
    fields.push_back(condition);
    fields.push_back(std::to_string(layoutSeed));
    fields.push_back(std::to_string(runSeed));
    fields.push_back(result.run.outcome);
    fields.push_back(std::to_string(result.decisionSteps));
    fields.push_back(std::to_string(result.run.worldSteps));
    fields.push_back(std::to_string(result.run.safeStops));
    fields.push_back(std::to_string(result.score.justifiedSafeStops));
    fields.push_back(std::to_string(result.score.mismatches));
    fields.push_back(std::to_string(result.score.mismatchesWithoutCause));
    fields.push_back(std::to_string(result.score.labelsCorrect));
    fields.push_back(std::to_string(result.score.labelsScored));
    fields.push_back(std::to_string(result.score.claims));
    fields.push_back(std::to_string(result.score.falseAlarms));
    fields.push_back(std::to_string(result.score.wrongAccepted));
    fields.push_back(std::to_string(result.run.evaluations));
    fields.push_back(std::to_string(result.activations.size()));
    fields.push_back(std::to_string(activated));
    return joinFields(fields);
}

bool writeTextFile(const std::string& path, const std::string& text) {
    std::ofstream out(path, std::ios::binary);
    if (!out) {
        return false;
    }
    out << text;
    out.flush();
    return out.good();
}

}  // namespace aih
