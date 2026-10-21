// Purpose: Runs the Stage 4 demonstration of the
// measuring instrument on demonstration material only.
// Introduced in: Stage 4f (rebuild)
// Architecture: This is the measuring instrument working
// on demonstration material only; it is not an experiment
// and its numbers are never compared. Layouts and fault
// events come from src/experiment/, the runs use
// runWithFaults, and the CSV lines use csv_output. Only
// demonstration seeds 500000-500003 are used. These lines
// show the instrument working; they are not results.

#include "demo/fault_demo.h"

#include <array>
#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

#include "demo/fault_demo_runs.h"
#include "experiment/csv_output.h"
#include "experiment/fault_event.h"
#include "experiment/fault_events.h"
#include "experiment/fault_injector.h"
#include "world/world_text.h"

namespace aih {
namespace {

// What follows one activation record in a run line: its
// step, its delay after deferral, or its refusal reason.
std::string activationSuffix(const ActivationRecord& record) {
    const std::string text = reportedAs(record);
    if (text == "Activated at its scheduled step") {
        return " (step " + std::to_string(record.applicationStep) + ")";
    }
    if (text == "Activated after deferral") {
        return " (step " + std::to_string(record.applicationStep) + ", delay " +
               std::to_string(activationDelay(record)) + ")";
    }
    if (!record.reason.empty()) {
        return " (" + record.reason + ")";
    }
    return "";
}

// The layout 500000 heading and its drawing, using the
// Stage 3 drawing helper (renderWorld, rule R4).
void writeLayout(std::ostream& out, const FaultDemoMaterial& material) {
    out << '\n';
    out << "Layout 500000 (found at attempt " << material.layout500000.attempts
        << ", shortest solution " << material.layout500000.shortest
        << " world steps):\n";
    const std::vector<std::string> lines =
        renderWorld(material.layout500000.world);
    for (const std::string& line : lines) {
        out << line << '\n';
    }
}

// The six events of runs 500000-1 and 500003-1.
void writeFaultEvents(std::ostream& out, const FaultDemoMaterial& material) {
    out << '\n';
    out << "Fault events of runs 500000-1 and 500003-1 (fault master seed "
           "4242):\n";
    const std::array<std::size_t, 2> indexes{{0, 6}};
    for (const std::size_t index : indexes) {
        for (const FaultEvent& event : material.events[index]) {
            out << faultEventText(event) << '\n';
        }
    }
}

// One run's heading and its three activation lines.
void writeDemoRun(std::ostream& out, const FaultDemoRun& run) {
    out << '\n';
    out << "Run " << run.layoutSeed << "-" << run.runSeed << ", "
        << run.condition << ": " << run.result.run.outcome << " after "
        << run.result.run.worldSteps << " world steps, "
        << run.result.decisionSteps << " decision steps, "
        << run.result.run.safeStops << " safe stops, "
        << run.result.run.evaluations << " model evaluations\n";
    for (std::size_t i = 0; i < run.events.size(); i = i + 1) {
        const FaultEvent& event = run.events[i];
        const ActivationRecord& record = run.result.activations[i];
        out << "  " << event.id << " " << faultTypeName(event.type) << ": "
            << reportedAs(record) << activationSuffix(record) << '\n';
    }
}

// The two CSV sections: one row per run, then one row
// per event of those runs.
void writeCsvSections(std::ostream& out,
                      const std::vector<FaultDemoRun>& runs) {
    out << '\n';
    out << "runs.csv:\n";
    out << runCsvHeader() << '\n';
    for (const FaultDemoRun& run : runs) {
        out << runCsvRow(run.condition, run.layoutSeed, run.runSeed, run.result)
            << '\n';
    }
    out << '\n';
    out << "events.csv:\n";
    out << eventCsvHeader() << '\n';
    for (const FaultDemoRun& run : runs) {
        for (std::size_t i = 0; i < run.events.size(); i = i + 1) {
            out << eventCsvRow(run.condition, run.events[i],
                               run.result.activations[i],
                               run.result.eventScores[i])
                << '\n';
        }
    }
}

}  // namespace

void runFaultDemo(std::ostream& out) {
    const FaultDemoMaterial material = faultDemoMaterial();
    const std::vector<FaultDemoRun> runs = collectFaultDemoRuns(material);
    out << '\n';
    out << "Stage 4: the measuring instrument, on demonstration material "
           "(not an experiment)\n";
    out << "Faults are injected into a reasoner's beliefs or reasoning, "
           "never into the world.\n";
    out << "Demonstration seeds 500000-500003 only; no pilot or "
           "confirmatory layout is used.\n";
    writeLayout(out, material);
    writeFaultEvents(out, material);
    for (const FaultDemoRun& run : runs) {
        writeDemoRun(out, run);
    }
    writeCsvSections(out, runs);
    out << '\n';
    out << "These lines show the instrument working; they are not results "
           "and are never compared.\n";
}

}  // namespace aih
