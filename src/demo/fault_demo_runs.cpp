// Purpose: Builds the demonstration material and runs
// its four demonstration runs with faults.
// Introduced in: Stage 4f (rebuild)
// Architecture: This follows the Python reference's
// demo_material() and demo_run(): the layout of each seed
// is generated once and repeated for its two run seeds,
// the event list is generated once for all eight runs,
// and runs 500000-2 and 500003-1 are run for C1 and C0
// with the illustrative settings of the demonstrations.
// These are demonstration runs, not results.

#include "demo/fault_demo_runs.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>

#include "agent/agent_types.h"
#include "agent/naive_agent.h"
#include "agent/observation.h"
#include "agent/single_agent.h"
#include "experiment/fault_events.h"
#include "experiment/fault_run.h"
#include "experiment/layout_generator.h"
#include "world/world_state.h"

namespace aih {
namespace {

// The first observation of a run in a true world.
Observation firstObservation(const WorldState& world) {
    Observation first;
    first.decisionStep = 1;
    first.state = world;
    return first;
}

// One C1 demonstration run on the given material index.
FaultDemoRun runC1Demo(const FaultDemoMaterial& material, std::size_t index) {
    FaultDemoRun run;
    run.condition = "C1";
    run.layoutSeed = material.events[index][0].layoutSeed;
    run.runSeed = material.events[index][0].runSeed;
    run.events = material.events[index];
    const AgentSettings settings{200, 5, 100, 3};
    SingleAgent agent(firstObservation(material.layouts[index]), settings);
    run.result = runWithFaults(material.layouts[index], agent, settings,
                               run.events, true);
    return run;
}

// One C0 demonstration run on the given material index.
FaultDemoRun runC0Demo(const FaultDemoMaterial& material, std::size_t index) {
    FaultDemoRun run;
    run.condition = "C0";
    run.layoutSeed = material.events[index][0].layoutSeed;
    run.runSeed = material.events[index][0].runSeed;
    run.events = material.events[index];
    const AgentSettings settings{200, 5, 100, 3};
    NaiveAgent agent(firstObservation(material.layouts[index]), settings);
    run.result = runWithFaults(material.layouts[index], agent, settings,
                               run.events, true);
    return run;
}

}  // namespace

FaultDemoMaterial faultDemoMaterial() {
    FaultDemoMaterial material;
    std::vector<RunKey> runs;
    for (std::uint64_t seed = 500000; seed <= 500003; seed = seed + 1) {
        const GeneratedLayout generated = generateLayout(seed);
        if (seed == 500000) {
            material.layout500000 = generated;
        }
        for (int runSeed = 1; runSeed <= 2; runSeed = runSeed + 1) {
            runs.push_back(RunKey{seed, runSeed});
            material.layouts.push_back(generated.world);
        }
    }
    material.events = generateFaultEvents(runs, material.layouts, 4242);
    return material;
}

std::vector<FaultDemoRun> collectFaultDemoRuns(
    const FaultDemoMaterial& material) {
    std::vector<FaultDemoRun> runs;
    const std::array<std::size_t, 2> indexes{{1, 6}};
    for (const std::size_t index : indexes) {
        runs.push_back(runC1Demo(material, index));
        runs.push_back(runC0Demo(material, index));
    }
    return runs;
}

}  // namespace aih
