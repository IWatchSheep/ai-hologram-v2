// Purpose: Declares CSV output for runs and fault events,
// and a helper that writes text to a file (plan 3.11).
// Introduced in: Stage 4f (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. It only reads the records made by the
// run with faults and the scorer; no agent file includes
// this header, and it changes no agent state. One CSV
// file holds one row per run (runs.csv) and the other
// holds one row per fault event (events.csv).

#ifndef AI_HOLOGRAM_SRC_EXPERIMENT_CSV_OUTPUT_H
#define AI_HOLOGRAM_SRC_EXPERIMENT_CSV_OUTPUT_H

#include <cstdint>
#include <string>

#include "experiment/fault_event.h"
#include "experiment/fault_injector.h"
#include "experiment/fault_run.h"
#include "experiment/scorer_types.h"

namespace aih {

// The first line of runs.csv, naming its columns.
std::string runCsvHeader();

// The first line of events.csv, naming its columns.
std::string eventCsvHeader();

// One runs.csv row: the run's outcome and its run score.
std::string runCsvRow(const std::string& condition, std::uint64_t layoutSeed,
                      int runSeed, const FaultRunResult& result);

// One events.csv row: what was scheduled for one event
// and what was measured for it. An event that was never
// activated has empty measurement fields, not zeros:
// empty means "did not happen", while 0 would look like
// a measured miss (D-044).
std::string eventCsvRow(const std::string& condition, const FaultEvent& event,
                        const ActivationRecord& activation,
                        const EventScore& score);

// Writes text to a file, exactly as given, and reports
// success. Binary mode keeps "\n" as "\n" on Windows.
// Stage 4 writes no result files into the repository: the
// demonstration prints its CSV lines to the screen.
bool writeTextFile(const std::string& path, const std::string& text);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_EXPERIMENT_CSV_OUTPUT_H
