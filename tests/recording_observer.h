// Purpose: Declares the one recording observer of the
// tests: a watch-only observer that records every report
// it receives, so a test can compare the reports exactly.
// Introduced in: Stage 4e (rebuild)
// Architecture: This is the tests' one copy of a recording
// observer (rule R4). It moved here from
// test_dead_square_observers.cpp, which now uses it too,
// and it also records evaluations. The observer only
// watches (rule R6): it records const reports and returns
// nothing any agent could read back.

#ifndef AI_HOLOGRAM_TESTS_RECORDING_OBSERVER_H
#define AI_HOLOGRAM_TESTS_RECORDING_OBSERVER_H

#include <string>
#include <vector>

#include "agent/computation_observer.h"
#include "world/position.h"

namespace aih {

// Everything one recording observer has seen: the dead
// cells and values read, the heuristic values reported,
// how many charged evaluations were reported, and every
// report as one line of text (the form the dead-square
// observer tests compare).
struct RecordingObserver {
    std::vector<Position> cells;
    std::vector<bool> values;
    std::vector<int> heuristics;
    int evaluations = 0;
    std::vector<std::string> events;

    // The observer to attach. Its functions capture this
    // recorder, so the recorder must outlive every use of
    // the observer built from it.
    ComputationObserver observer();
};

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_RECORDING_OBSERVER_H
