// Purpose: Tests for the whole layout sets (LY5, LY6, plan
// Section 6.1; material only, decision D-080).
// Introduced in: Stage 4a (rebuild)
// Architecture: LY5 generates the pilot and confirmatory
// layout material only: no agent is run on it, so its balance
// can be checked without breaking rule R21. LY6 shows the
// same seed always generates the identical layout.

#include <cstdint>

#include "experiment/layout_generator.h"
#include "test_suites.h"
#include "test_support.h"

namespace aih {
namespace {

// The summary of one generated set: whether every layout was
// found, and the largest attempts and shortest values seen.
struct SetSummary {
    bool allFound = true;
    int largestAttempts = 0;
    int smallestShortest = 0;
    int largestShortest = 0;
};

// Generates count layouts from firstSeed and summarises them.
SetSummary summariseSet(std::uint64_t firstSeed, int count) {
    SetSummary summary;
    summary.smallestShortest = 1000;
    for (int i = 0; i < count; i = i + 1) {
        const std::uint64_t seed = firstSeed + static_cast<std::uint64_t>(i);
        const GeneratedLayout layout = generateLayout(seed);
        if (!layout.found) {
            summary.allFound = false;
        }
        if (layout.attempts > summary.largestAttempts) {
            summary.largestAttempts = layout.attempts;
        }
        if (layout.shortest < summary.smallestShortest) {
            summary.smallestShortest = layout.shortest;
        }
        if (layout.shortest > summary.largestShortest) {
            summary.largestShortest = layout.shortest;
        }
    }
    return summary;
}

void checkSets(TestCounter& counter) {
    const SetSummary pilot = summariseSet(900000, 40);
    const bool pilotOk = pilot.allFound && pilot.largestAttempts == 10 &&
                         pilot.smallestShortest == 12 &&
                         pilot.largestShortest == 35;
    check(counter, pilotOk,
          "LY5: pilot material found, attempts and shortest range");
    const SetSummary confirmatory = summariseSet(100000, 400);
    const bool confirmatoryOk = confirmatory.allFound &&
                                confirmatory.largestAttempts == 20 &&
                                confirmatory.smallestShortest == 12 &&
                                confirmatory.largestShortest == 38;
    check(counter, confirmatoryOk,
          "LY5: confirmatory material found, attempts and range");
}

void checkRepeat(TestCounter& counter) {
    const GeneratedLayout first = generateLayout(500002);
    const GeneratedLayout second = generateLayout(500002);
    const bool same =
        first.found == second.found && first.world == second.world &&
        first.attempts == second.attempts && first.shortest == second.shortest;
    check(counter, same, "LY6: generating 500002 twice is identical");
}

}  // namespace

void runLayoutSetsTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Layout sets ---\n";
    }
    checkSets(counter);
    checkRepeat(counter);
}

}  // namespace aih
