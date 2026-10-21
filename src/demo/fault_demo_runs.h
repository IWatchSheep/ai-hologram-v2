// Purpose: Declares the demonstration material and its
// four runs, shared by the Stage 4 demonstration text.
// Introduced in: Stage 4f (rebuild)
// Architecture: The material is built here once, in the
// way the Python reference builds its demo_material(), so
// the demonstration text and its runs can never be built
// in two different ways. Only demonstration seeds
// 500000-500003 are used; no pilot or confirmatory layout
// is used, and no agent is run on one.

#ifndef AI_HOLOGRAM_SRC_DEMO_FAULT_DEMO_RUNS_H
#define AI_HOLOGRAM_SRC_DEMO_FAULT_DEMO_RUNS_H

#include <cstdint>
#include <string>
#include <vector>

#include "experiment/fault_event.h"
#include "experiment/fault_run.h"
#include "experiment/layout_generator.h"
#include "world/world_state.h"

namespace aih {

// The demonstration material: layout 500000 on its own,
// the true layout of each of the eight runs, and the
// generated event list, one list of 3 events per run.
struct FaultDemoMaterial {
    GeneratedLayout layout500000;
    std::vector<WorldState> layouts;
    std::vector<std::vector<FaultEvent>> events;
};

// One demonstration run: its condition, its run key, its
// events and the result of its run with faults.
struct FaultDemoRun {
    std::string condition;
    std::uint64_t layoutSeed = 0;
    int runSeed = 0;
    std::vector<FaultEvent> events;
    FaultRunResult result;
};

// Builds the demonstration material: layouts 500000-500003,
// run seeds 1-2 each, fault master seed 4242.
FaultDemoMaterial faultDemoMaterial();

// The four demonstration runs, in demonstration order:
// runs 500000-2 and 500003-1, C1 then C0 for each.
std::vector<FaultDemoRun> collectFaultDemoRuns(
    const FaultDemoMaterial& material);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_DEMO_FAULT_DEMO_RUNS_H
