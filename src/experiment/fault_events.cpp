// Purpose: Generates the fault-event list of plan 3.3,
// draw by draw as the reference does.
// Introduced in: Stage 4b (rebuild)
// Architecture: Two random streams are used on purpose. The
// experiment-level stream shuffles the fault types once and
// then, type by type, decides the targets and the F1
// directions, so the balance is exact over the whole
// experiment. Each run then draws from its own stream, so
// changing one run never changes another run's events. One
// reordered draw would change every later number.

#include "experiment/fault_events.h"

#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>

#include "agent/dead_squares.h"
#include "agent/directions.h"
#include "experiment/seeded_random.h"
#include "world/position.h"

namespace aih {
namespace {

// The fault type for i mod 5: 0 gives F1, up to 4 gives F5.
// The enum is declared in that order, so the number is the type.
FaultType faultTypeForRemainder(int remainder) {
    return static_cast<FaultType>(remainder);
}

// Builds the events in run order with their identifiers.
// Run r holds events 3r, 3r + 1 and 3r + 2, index 1, 2, 3.
std::vector<FaultEvent> makeEvents(const std::vector<RunKey>& runs,
                                   const std::vector<FaultType>& types) {
    std::vector<FaultEvent> events;
    for (std::size_t i = 0; i < types.size(); i = i + 1) {
        const RunKey& run = runs[i / 3];
        FaultEvent event;
        event.layoutSeed = run.layoutSeed;
        event.runSeed = run.runSeed;
        event.index = static_cast<int>(i % 3) + 1;
        event.type = types[i];
        event.id = std::to_string(run.layoutSeed) + "-" +
                   std::to_string(run.runSeed) + "-" +
                   std::to_string(event.index);
        events.push_back(event);
    }
    return events;
}

// Gives every event of one type its target, from a shuffled
// copy of that type's event numbers: the first
// commonModeCount are common-mode (both), the rest alternate
// R1, R2. For F1, a second shuffled copy alternates flips.
void assignTypeTargets(std::vector<FaultEvent>& events, FaultType type,
                       SeededRandom& random) {
    std::vector<int> perm;
    for (std::size_t i = 0; i < events.size(); i = i + 1) {
        if (events[i].type == type) {
            perm.push_back(static_cast<int>(i));
        }
    }
    const std::vector<int> idx = perm;
    random.shuffle(perm);
    const int common = commonModeCount(static_cast<int>(idx.size()));
    for (std::size_t p = 0; p < perm.size(); p = p + 1) {
        FaultEvent& event = events[static_cast<std::size_t>(perm[p])];
        const bool isCommon = static_cast<int>(p) < common;
        const int rest = static_cast<int>(p) - common;
        event.commonMode = isCommon;
        event.target =
            isCommon ? FaultTarget::Both
                     : (rest % 2 == 0 ? FaultTarget::R1 : FaultTarget::R2);
    }
    if (type == FaultType::F1) {
        std::vector<int> perm2 = idx;
        random.shuffle(perm2);
        for (std::size_t p = 0; p < perm2.size(); p = p + 1) {
            FaultEvent& event = events[static_cast<std::size_t>(perm2[p])];
            event.direction = p % 2 == 0 ? F1Direction::FreeToBlocked
                                         : F1Direction::BlockedToFree;
        }
    }
}

// The candidate cells for an F1 or F4 event: interior cells
// (y 1..8 outer, x 1..8 inner) other than the destination.
// F1 wants a true wall exactly when its flip is
// blocked->free; F4 wants a true dead cell (no overrides).
std::vector<Position> candidatesFor(const FaultEvent& event,
                                    const WorldState& layout) {
    FreeComputationCounts counts;
    DeadSquareMap deadMap;
    if (event.type == FaultType::F4) {
        deadMap = computeDeadSquares(layout, {}, {}, counts);
    }
    std::vector<Position> candidates;
    for (int y = 1; y <= 8; y = y + 1) {
        for (int x = 1; x <= 8; x = x + 1) {
            const Position cell{x, y};
            if (cell == layout.destination) {
                continue;
            }
            const std::size_t row = static_cast<std::size_t>(y);
            const std::size_t column = static_cast<std::size_t>(x);
            bool wanted = false;
            if (event.type == FaultType::F4) {
                wanted = deadMap.dead[row][column];
            }
            if (event.type == FaultType::F1) {
                const bool isWall = layout.walls[row][column];
                wanted =
                    (event.direction == F1Direction::BlockedToFree) == isWall;
            }
            if (wanted) {
                candidates.push_back(cell);
            }
        }
    }
    return candidates;
}

// Fills one event's detail from the run's own stream. F1
// and F4 draw one candidate cell, or make no draw when there
// is none; F2 shuffles the direction order; F3 draws an
// action then a rotation; F5 draws nothing.
void fillEventDetail(FaultEvent& event, const WorldState& layout,
                     SeededRandom& random) {
    if (event.type == FaultType::F1 || event.type == FaultType::F4) {
        const std::vector<Position> cells = candidatesFor(event, layout);
        if (!cells.empty()) {
            event.hasCell = true;
            const int chosen = random.below(static_cast<int>(cells.size()));
            event.cell = cells[static_cast<std::size_t>(chosen)];
        }
    }
    if (event.type == FaultType::F2) {
        std::vector<Action> order(kActionOrder.begin(), kActionOrder.end());
        random.shuffle(order);
        std::copy(order.begin(), order.end(), event.order.begin());
    }
    if (event.type == FaultType::F3) {
        event.action = kActionOrder[static_cast<std::size_t>(random.below(4))];
        event.rotation =
            random.below(2) == 0 ? Rotation::Left : Rotation::Right;
    }
}

}  // namespace

int commonModeCount(int count) {
    return (2 * count + 5) / 10;
}

std::uint64_t runStreamSeed(std::uint64_t masterSeed, std::uint64_t layoutSeed,
                            int runSeed) {
    return (masterSeed << 40) + (layoutSeed << 8) +
           static_cast<std::uint64_t>(runSeed);
}

std::vector<std::vector<FaultEvent>> generateFaultEvents(
    const std::vector<RunKey>& runs, const std::vector<WorldState>& layouts,
    std::uint64_t masterSeed) {
    if (runs.size() != layouts.size()) {
        return {};
    }
    SeededRandom experimentRandom(masterSeed);
    std::vector<FaultType> types;
    const int eventCount = 3 * static_cast<int>(runs.size());
    for (int i = 0; i < eventCount; i = i + 1) {
        types.push_back(faultTypeForRemainder(i % 5));
    }
    experimentRandom.shuffle(types);
    std::vector<FaultEvent> events = makeEvents(runs, types);
    for (int i = 0; i < 5; i = i + 1) {
        assignTypeTargets(events, faultTypeForRemainder(i), experimentRandom);
    }
    std::vector<std::vector<FaultEvent>> result;
    for (std::size_t r = 0; r < runs.size(); r = r + 1) {
        SeededRandom runRandom(
            runStreamSeed(masterSeed, runs[r].layoutSeed, runs[r].runSeed));
        std::vector<int> steps = runRandom.pickDistinct(
            std::vector<int>{1, 2, 3, 4, 5, 6, 7, 8, 9, 10}, 3);
        std::sort(steps.begin(), steps.end());
        std::vector<FaultEvent> runEvents;
        for (std::size_t j = 0; j < 3; j = j + 1) {
            FaultEvent& event = events[3 * r + j];
            event.step = steps[j];
            fillEventDetail(event, layouts[r], runRandom);
            runEvents.push_back(event);
        }
        result.push_back(runEvents);
    }
    return result;
}

}  // namespace aih
