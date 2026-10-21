// Purpose: Tests for the fault-event lists of the pilot and
// confirmatory material (FE2, FE3, and FE4 for those sets;
// plan Section 6.1).
// Introduced in: Stage 4b (rebuild)
// Architecture: These checks generate MATERIAL ONLY (layouts
// and event lists, D-080): no agent is run on the pilot or
// confirmatory layouts. Expected values are copied from
// checks.py check_events.

#include <cstddef>
#include <string>
#include <vector>

#include "experiment/fault_event.h"
#include "experiment/fault_events.h"
#include "fault_test_material.h"
#include "test_suites.h"
#include "test_support.h"

namespace aih {
namespace {

// The five fault types in their fixed order.
std::vector<FaultType> allTypes() {
    return {FaultType::F1, FaultType::F2, FaultType::F3, FaultType::F4,
            FaultType::F5};
}

// The text of every event of one run.
std::vector<std::string> runTexts(const std::vector<FaultEvent>& run) {
    std::vector<std::string> texts;
    for (const FaultEvent& event : run) {
        texts.push_back(faultEventText(event));
    }
    return texts;
}

// True when every type has perType events, of which common
// are common-mode and perTarget are aimed at each reasoner.
bool typeCountsOk(const FaultTestMaterial& material, int perType, int common,
                  int perTarget) {
    const std::vector<FaultEvent> flat = flattenEvents(material.events);
    bool ok = true;
    for (const FaultType type : allTypes()) {
        int total = 0;
        int both = 0;
        int r1 = 0;
        int r2 = 0;
        for (const FaultEvent& event : flat) {
            if (event.type == type) {
                total = total + 1;
                if (event.target == FaultTarget::Both) {
                    both = both + 1;
                }
                if (event.target == FaultTarget::R1) {
                    r1 = r1 + 1;
                }
                if (event.target == FaultTarget::R2) {
                    r2 = r2 + 1;
                }
            }
        }
        ok = ok && total == perType && both == common;
        ok = ok && r1 == perTarget && r2 == perTarget;
    }
    return ok;
}

// True when every F1 and F4 event has a cell, and the F1
// directions are split exactly half and half.
bool directionsOk(const FaultTestMaterial& material, int half) {
    bool cells = true;
    int freeToBlocked = 0;
    int blockedToFree = 0;
    for (const FaultEvent& event : flattenEvents(material.events)) {
        if (event.type == FaultType::F1 || event.type == FaultType::F4) {
            cells = cells && event.hasCell;
        }
        if (event.type == FaultType::F1) {
            if (event.direction == F1Direction::FreeToBlocked) {
                freeToBlocked = freeToBlocked + 1;
            }
            if (event.direction == F1Direction::BlockedToFree) {
                blockedToFree = blockedToFree + 1;
            }
        }
    }
    return cells && freeToBlocked == half && blockedToFree == half;
}

void checkPilot(TestCounter& counter, const FaultTestMaterial& pilot) {
    const bool counts =
        pilot.events.size() == 80 && typeCountsOk(pilot, 48, 10, 19);
    check(counter, counts, "FE2: pilot counts per type");
    check(counter, directionsOk(pilot, 24),
          "FE2: pilot F1 directions 24/24 and every F1/F4 has a cell");
    const std::vector<std::string> expected{
        "900000-1-1 step 1 F4 target both | (4, 1)",
        "900000-1-2 step 2 F5 target both | -",
        "900000-1-3 step 8 F3 target both | West right"};
    check(counter, runTexts(pilot.events[0]) == expected,
          "FE2: run 900000-1's lines");
    check(counter, faultListValid(pilot),
          "FE4: pilot list steps and cells valid");
}

// How many events are scheduled at each step 1 to 10.
std::vector<int> eventsPerStep(const FaultTestMaterial& material) {
    std::vector<int> steps(10, 0);
    for (const FaultEvent& event : flattenEvents(material.events)) {
        const std::size_t at = static_cast<std::size_t>(event.step - 1);
        steps[at] = steps[at] + 1;
    }
    return steps;
}

void checkConfirmatory(TestCounter& counter,
                       const FaultTestMaterial& confirmatory) {
    const bool counts = confirmatory.events.size() == 2000 &&
                        typeCountsOk(confirmatory, 1200, 240, 480);
    check(counter, counts, "FE3: confirmatory counts per type");
    check(counter, directionsOk(confirmatory, 600),
          "FE3: confirmatory directions 600/600 and cells");
    const std::vector<int> expectedSteps{580, 620, 603, 578, 609,
                                         577, 614, 597, 605, 617};
    check(counter, eventsPerStep(confirmatory) == expectedSteps,
          "FE3: confirmatory events per step");
    const std::vector<std::string> first{
        "100000-1-1 step 2 F2 target both | South West East North",
        "100000-1-2 step 7 F2 target both | South North East West",
        "100000-1-3 step 8 F3 target R2 | South right"};
    const std::vector<std::string> last{
        "100399-5-1 step 2 F5 target R2 | -",
        "100399-5-2 step 5 F4 target R1 | (8, 3)",
        "100399-5-3 step 6 F1 target both | (1, 3) free->blocked"};
    const bool ends = runTexts(confirmatory.events.front()) == first &&
                      runTexts(confirmatory.events.back()) == last;
    check(counter, ends, "FE3: first and last runs' lines");
    check(counter, faultListValid(confirmatory),
          "FE4: confirmatory list steps and cells valid");
}

}  // namespace

void runFaultEventSetsTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Fault event sets ---\n";
    }
    const FaultTestMaterial pilot = buildFaultMaterial(900000, 40, 2, 4242);
    checkPilot(counter, pilot);
    const FaultTestMaterial confirmatory =
        buildFaultMaterial(100000, 400, 5, 4242);
    checkConfirmatory(counter, confirmatory);
}

}  // namespace aih
