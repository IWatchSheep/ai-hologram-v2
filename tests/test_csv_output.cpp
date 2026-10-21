// Purpose: Tests for CSV output (CSV1, CSV3, CSV4; plan
// Section 6.5): the two headers, one not-activated event
// row, and the file-writing helper.
// Introduced in: Stage 4f (rebuild)
// Architecture: Expected values are copied from checks.py
// section 4f. CSV4 writes only in the system temporary
// folder, never in the repository, reads its file back,
// and removes it, so no test leaves a file behind.

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include "agent/single_agent.h"
#include "agent_test_fixtures.h"
#include "experiment/csv_output.h"
#include "experiment/fault_run.h"
#include "fault_test_material.h"
#include "stage4_suites.h"
#include "test_support.h"

namespace aih {
namespace {

// The two header lines of checks.py section 4f.
void checkCsv1(TestCounter& counter) {
    const std::string runHeader =
        "run_id,condition,layout_seed,run_seed,outcome,decision_steps,"
        "world_steps,safe_stops,justified_safe_stops,mismatches,"
        "mismatches_without_fault_cause,source_labels_correct,"
        "source_labels_scored,claims,false_alarms,"
        "wrong_accepted_corrections,evaluations,events_scheduled,"
        "events_activated";
    check(counter, runCsvHeader() == runHeader, "CSV1: the run header");
    const std::string eventHeader =
        "event_id,condition,reasoner,layout_seed,run_seed,type,target,"
        "common_mode,cell,direction,order,action,rotation,scheduled_step,"
        "reported_as,activation_step,activation_delay,"
        "not_activated_reason,opportunity,opportunity_step,"
        "revealing_opportunity,mismatch_detected,mismatch_step,"
        "strictly_detected,strict_step,detection_delay,end_state,"
        "end_step,correction_status,verified_correction";
    check(counter, eventCsvHeader() == eventHeader, "CSV1: the event header");
}

// CSV3: event 500001-1-3 is never activated, so every
// measurement field of its row is empty (D-044). Its
// reported_as text contains a comma (quirk Q7, D-078),
// written unquoted exactly as the original wrote it.
void checkCsv3(TestCounter& counter) {
    const FaultTestMaterial material = demonstrationFaultMaterial();
    SingleAgent agent(makeObservation(1, material.layouts[2]), AgentSettings{});
    const FaultRunResult run = runWithFaults(
        material.layouts[2], agent, AgentSettings{}, material.events[2]);
    const std::string row =
        "500001-1-3,C1,R,500001,1,F5,R2,0,,,,,,8,Not activated: deferred, "
        "never applicable,,,the object has not moved yet,,,,,,,,,,,,";
    bool ok = run.activations.size() == 3 && run.eventScores.size() == 3;
    if (ok) {
        ok = eventCsvRow("C1", material.events[2][2], run.activations[2],
                         run.eventScores[2]) == row;
    }
    check(counter, ok,
          "CSV3: event 500001-1-3 (C1, not activated) row exactly");
    const bool detached = agent.state.deadCache.observer == nullptr &&
                          agent.state.deadCache.map.observer == nullptr;
    check(counter, detached,
          "CSV3: after the run the cache and cached-map observers are null");
}

// Reads a whole file back as text, exactly as written.
std::string readTextFile(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    std::ostringstream text;
    text << in.rdbuf();
    return text.str();
}

// CSV4: writeTextFile in the system temporary folder.
void checkCsv4(TestCounter& counter) {
    const std::filesystem::path path =
        std::filesystem::temp_directory_path() / "stage4_csv_test.csv";
    const std::string text = "a,b\n1,2\n";
    const bool wrote = writeTextFile(path.string(), text);
    check(counter, wrote && std::filesystem::exists(path),
          "CSV4: writeTextFile writes a file in the temporary folder");
    check(counter, readTextFile(path) == text,
          "CSV4: the file reads back identical");
    const bool removed = std::filesystem::remove(path);
    check(counter, removed && !std::filesystem::exists(path),
          "CSV4: the file is removed, nothing is left behind");
    const std::filesystem::path missing =
        std::filesystem::temp_directory_path() / "no_such_folder_stage4" /
        "file.csv";
    check(counter, !writeTextFile(missing.string(), text),
          "CSV4: a missing folder returns false");
}

}  // namespace

void runCsvOutputTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- CSV output ---\n";
    }
    checkCsv1(counter);
    checkCsv3(counter);
    checkCsv4(counter);
}

}  // namespace aih
