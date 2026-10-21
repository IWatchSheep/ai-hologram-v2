// Purpose: Declares the shared fault-test material: the
// runs, layouts and event lists the Stage 4b tests share
// (one shared helper per job, rule R4).
// Introduced in: Stage 4b (rebuild)
// Architecture: The demonstration material is built here
// once, in the way checks.py builds its material(), so the
// two fault-event test files can never build it in two
// different ways. Generating pilot and confirmatory material
// here is allowed (D-080): no agent is ever run on it.

#ifndef AI_HOLOGRAM_TESTS_FAULT_TEST_MATERIAL_H
#define AI_HOLOGRAM_TESTS_FAULT_TEST_MATERIAL_H

#include <cstdint>
#include <string>
#include <vector>

#include "experiment/fault_event.h"
#include "experiment/fault_events.h"
#include "experiment/fault_injector.h"
#include "world/world_state.h"

namespace aih {

// One experiment's material: the runs in order (layout seed,
// then run seed), the true layout of each run, and the
// generated event list, one list of 3 events per run.
struct FaultTestMaterial {
    std::vector<RunKey> runs;
    std::vector<WorldState> layouts;
    std::vector<std::vector<FaultEvent>> events;
};

// Builds the material for layout seeds firstLayoutSeed to
// firstLayoutSeed + layoutCount - 1, each with run seeds 1
// to runSeedCount, under the given master seed. Layouts come
// from generateLayout, and the event list from
// generateFaultEvents, exactly as checks.py material() does.
FaultTestMaterial buildFaultMaterial(std::uint64_t firstLayoutSeed,
                                     int layoutCount, int runSeedCount,
                                     std::uint64_t masterSeed);

// The demonstration material: layout seeds 500000-500003,
// run seeds 1-2, master seed 4242.
FaultTestMaterial demonstrationFaultMaterial();

// All events of an event list as one flat list, in run order.
std::vector<FaultEvent> flattenEvents(
    const std::vector<std::vector<FaultEvent>>& events);

// One hand-built event, like checks.py event(): the id,
// step and type are set, and every detail field keeps its
// default until the test sets the ones its type needs.
FaultEvent handBuiltEvent(const std::string& id, int step, FaultType type);

// The activation record of one event id, like checks.py
// record(): the first record with that id. Tests only ask
// for ids that exist; a missing id gives an empty record,
// so the check that uses it fails instead of crashing.
const ActivationRecord& findActivation(
    const std::vector<ActivationRecord>& records, const std::string& eventId);

// The FE4 validity check of checks.py list_valid, shared by
// both fault-event test files (R4): 3 sorted distinct steps
// 1-10 per run, and every F1 or F4 cell interior, not the
// destination, with the true status its type requires.
bool faultListValid(const FaultTestMaterial& material);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_FAULT_TEST_MATERIAL_H
