// Purpose: Declares the layout generator that builds one
// experiment layout from one layout seed.
// Introduced in: Stage 4a (rebuild)
// Architecture: This is experimenter code in src/experiment/.
// A layout seed goes into SeededRandom, and the draws are
// made in a fixed order, so the same seed always builds the
// same layout on any machine. A layout is accepted only when
// it is a valid world with a shortest solution of 12 to 40
// steps and at least one true dead interior cell.

#ifndef AI_HOLOGRAM_SRC_EXPERIMENT_LAYOUT_GENERATOR_H
#define AI_HOLOGRAM_SRC_EXPERIMENT_LAYOUT_GENERATOR_H

#include <cstdint>

#include "world/world_state.h"

namespace aih {

// The result of generating one layout: whether an attempt
// was accepted, its world, how many attempts were needed
// (10000 when none was accepted), and the shortest solution
// length of the accepted world (0 when none was accepted).
struct GeneratedLayout {
    bool found = false;
    WorldState world{};
    int attempts = 0;
    int shortest = 0;
};

// Builds a layout from layoutSeed, trying at most 10000
// attempts. Each attempt draws interior walls row by row,
// then picks agent, object and destination from the free
// cells, and is checked in the order of plan Section 3.2.
GeneratedLayout generateLayout(std::uint64_t layoutSeed);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_EXPERIMENT_LAYOUT_GENERATOR_H
