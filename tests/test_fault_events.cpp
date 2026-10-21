// Purpose: Tests for the fault-event list on the
// demonstration material (FE1, FE4 for the demonstration
// list, FE5, FE6; plan Section 6.1).
// Introduced in: Stage 4b (rebuild)
// Architecture: Expected values are copied from checks.py
// check_events. The demonstration material comes from the
// shared fault_test_material (R4); no agent is run here,
// only the generated list is compared.

#include <cstdint>
#include <string>
#include <vector>

#include "experiment/fault_event.h"
#include "experiment/fault_events.h"
#include "fault_test_material.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/world_state.h"

namespace aih {
namespace {

// The 24 demonstration lines of checks.py DEMO_TEXTS, in
// order: layout seeds 500000-500003, run seeds 1-2.
std::vector<std::string> demoTexts() {
    return {"500000-1-1 step 4 F1 target both | (6, 8) free->blocked",
            "500000-1-2 step 6 F3 target R1 | East right",
            "500000-1-3 step 10 F4 target R2 | (8, 2)",
            "500000-2-1 step 2 F1 target R1 | (1, 6) blocked->free",
            "500000-2-2 step 9 F2 target R2 | North South East West",
            "500000-2-3 step 10 F4 target R2 | (7, 1)",
            "500001-1-1 step 4 F2 target R1 | West North East South",
            "500001-1-2 step 6 F2 target R2 | West East South North",
            "500001-1-3 step 8 F5 target R2 | -",
            "500001-2-1 step 2 F1 target R1 | (1, 3) free->blocked",
            "500001-2-2 step 4 F2 target R1 | West East South North",
            "500001-2-3 step 10 F4 target both | (7, 1)",
            "500002-1-1 step 3 F3 target R1 | East right",
            "500002-1-2 step 6 F4 target R1 | (4, 1)",
            "500002-1-3 step 7 F1 target R2 | (2, 8) blocked->free",
            "500002-2-1 step 5 F3 target both | South right",
            "500002-2-2 step 8 F3 target R2 | West left",
            "500002-2-3 step 9 F5 target both | -",
            "500003-1-1 step 1 F5 target R1 | -",
            "500003-1-2 step 2 F4 target R1 | (1, 7)",
            "500003-1-3 step 8 F1 target R2 | (6, 8) free->blocked",
            "500003-2-1 step 1 F2 target both | East North South West",
            "500003-2-2 step 9 F3 target R2 | North right",
            "500003-2-3 step 10 F5 target R1 | -"};
}

// The text of every event of a material, in run order.
std::vector<std::string> materialTexts(const FaultTestMaterial& material) {
    std::vector<std::string> texts;
    for (const FaultEvent& event : flattenEvents(material.events)) {
        texts.push_back(faultEventText(event));
    }
    return texts;
}

void checkDemoList(TestCounter& counter, const FaultTestMaterial& material) {
    check(counter, materialTexts(material) == demoTexts(),
          "FE1: the 24 demonstration lines");
    const std::vector<FaultEvent> flat = flattenEvents(material.events);
    bool commonOk = true;
    int commonCount = 0;
    bool idsOk = true;
    for (const FaultEvent& event : flat) {
        commonOk =
            commonOk && event.commonMode == (event.target == FaultTarget::Both);
        if (event.commonMode) {
            commonCount = commonCount + 1;
        }
        const std::string id = std::to_string(event.layoutSeed) + "-" +
                               std::to_string(event.runSeed) + "-" +
                               std::to_string(event.index);
        idsOk = idsOk && event.id == id && event.index >= 1 && event.index <= 3;
    }
    check(counter, commonOk && commonCount == 5,
          "FE1: common-mode exactly for the 5 both events");
    check(counter, idsOk, "FE1: ids match layout, run and index");
}

void checkRepeatAndSeeds(TestCounter& counter,
                         const FaultTestMaterial& material) {
    const FaultTestMaterial again = demonstrationFaultMaterial();
    const bool twice = materialTexts(again) == materialTexts(material) &&
                       materialTexts(material).size() == 24;
    check(counter, twice, "FE5: twice identical (24 lines)");
    const FaultTestMaterial other = buildFaultMaterial(500000, 4, 2, 4243);
    const std::string first = faultEventText(other.events[0][0]);
    check(counter, first == "500000-1-1 step 1 F3 target R1 | East left",
          "FE5: master seed 4243's first line");
    const std::vector<RunKey> oneRun{RunKey{500000, 1}};
    const std::vector<WorldState> noLayouts;
    const bool empty = generateFaultEvents(oneRun, noLayouts, 4242).empty();
    check(counter, empty,
          "FE5: different run and layout counts give an empty list");
}

void checkCountsAndSeed(TestCounter& counter) {
    const std::vector<int> counts{2, 3, 5, 8, 48, 1200};
    const std::vector<int> expected{0, 1, 1, 2, 10, 240};
    std::vector<int> actual;
    for (const int count : counts) {
        actual.push_back(commonModeCount(count));
    }
    check(counter, actual == expected, "FE6: commonModeCount's values");
    const std::uint64_t expectedSeed =
        (std::uint64_t{4242} << 40) + (std::uint64_t{500000} << 8) + 1;
    check(counter, runStreamSeed(4242, 500000, 1) == expectedSeed,
          "FE6: runStreamSeed(4242, 500000, 1)");
}

}  // namespace

void runFaultEventsTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Fault events ---\n";
    }
    const FaultTestMaterial material = demonstrationFaultMaterial();
    checkDemoList(counter, material);
    check(counter, faultListValid(material),
          "FE4: demonstration list steps and cells valid");
    checkRepeatAndSeeds(counter, material);
    checkCountsAndSeed(counter);
}

}  // namespace aih
