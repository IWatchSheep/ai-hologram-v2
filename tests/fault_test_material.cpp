// Purpose: Builds the shared fault-test material and the
// shared FE4 validity check.
// Introduced in: Stage 4b (rebuild)
// Architecture: buildFaultMaterial follows checks.py
// material() exactly: for each layout seed, the layout is
// generated once and repeated for each of its run seeds, and
// the whole event list is generated once for all the runs.

#include "fault_test_material.h"

#include <cstddef>
#include <cstdint>
#include <vector>

#include "agent/dead_squares.h"
#include "experiment/fault_event.h"
#include "experiment/layout_generator.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

// True when one event's cell (if it has one) is allowed for
// its type in this layout: interior, not the destination,
// and for F1 a true wall exactly when its flip is
// blocked->free, for F4 truly dead in the given map.
bool eventCellValid(const FaultEvent& event, const WorldState& layout,
                    const DeadSquareMap& deadMap) {
    if (!event.hasCell) {
        return true;
    }
    const bool inside = event.cell.x >= 1 && event.cell.x <= 8 &&
                        event.cell.y >= 1 && event.cell.y <= 8;
    if (!inside || event.cell == layout.destination) {
        return false;
    }
    const std::size_t row = static_cast<std::size_t>(event.cell.y);
    const std::size_t column = static_cast<std::size_t>(event.cell.x);
    if (event.type == FaultType::F1) {
        const bool wall = layout.walls[row][column];
        if (event.direction == F1Direction::BlockedToFree) {
            return wall;
        }
        return !wall;
    }
    if (event.type == FaultType::F4) {
        return deadMap.dead[row][column];
    }
    return true;
}

}  // namespace

FaultTestMaterial buildFaultMaterial(std::uint64_t firstLayoutSeed,
                                     int layoutCount, int runSeedCount,
                                     std::uint64_t masterSeed) {
    FaultTestMaterial material;
    for (int i = 0; i < layoutCount; i = i + 1) {
        const std::uint64_t seed =
            firstLayoutSeed + static_cast<std::uint64_t>(i);
        const GeneratedLayout layout = generateLayout(seed);
        for (int r = 1; r <= runSeedCount; r = r + 1) {
            material.runs.push_back(RunKey{seed, r});
            material.layouts.push_back(layout.world);
        }
    }
    material.events =
        generateFaultEvents(material.runs, material.layouts, masterSeed);
    return material;
}

FaultTestMaterial demonstrationFaultMaterial() {
    return buildFaultMaterial(500000, 4, 2, 4242);
}

FaultEvent handBuiltEvent(const std::string& id, int step, FaultType type) {
    FaultEvent event;
    event.id = id;
    event.step = step;
    event.type = type;
    return event;
}

const ActivationRecord& findActivation(
    const std::vector<ActivationRecord>& records, const std::string& eventId) {
    for (const ActivationRecord& record : records) {
        if (record.eventId == eventId) {
            return record;
        }
    }
    static const ActivationRecord empty;
    return empty;
}

std::vector<FaultEvent> flattenEvents(
    const std::vector<std::vector<FaultEvent>>& events) {
    std::vector<FaultEvent> flat;
    for (const std::vector<FaultEvent>& run : events) {
        for (const FaultEvent& event : run) {
            flat.push_back(event);
        }
    }
    return flat;
}

bool faultListValid(const FaultTestMaterial& material) {
    bool ok = material.events.size() == material.runs.size();
    ok = ok && material.layouts.size() == material.runs.size();
    if (!ok) {
        return false;
    }
    for (std::size_t r = 0; r < material.events.size(); r = r + 1) {
        const std::vector<FaultEvent>& run = material.events[r];
        const WorldState& layout = material.layouts[r];
        FreeComputationCounts counts;
        const DeadSquareMap deadMap =
            computeDeadSquares(layout, {}, {}, counts);
        ok = ok && run.size() == 3;
        for (std::size_t j = 0; j < run.size(); j = j + 1) {
            const FaultEvent& event = run[j];
            ok = ok && event.index == static_cast<int>(j) + 1;
            ok = ok && event.step >= 1 && event.step <= 10;
            if (j > 0) {
                ok = ok && run[j - 1].step < event.step;
            }
            ok = ok && eventCellValid(event, layout, deadMap);
        }
    }
    return ok;
}

}  // namespace aih
