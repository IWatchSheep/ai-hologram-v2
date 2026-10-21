// Prints, from the ORIGINAL C++ at 89ea9b6, the layouts and fault-event lists of the
// demonstration sets used by the Stage 4 rebuild's golden cases (plan STAGE_04 3.15):
//   demo   layouts 500000-500003 x run seeds 1-2 (the original demonstration experiment)
//   seeds  layouts 500000-500009 x run seeds 1-3 (M1.4's 60 further demonstration-seed runs)
// Reference material only (exempt from R1).
#include "experiment/fault_events.h"
#include "experiment/layout_generator.h"
#include "world/world_text.h"
#include <iostream>
#include <string>
using namespace ai_hologram;
int main(int argc, char** argv) {
    const std::string set = argc > 1 ? argv[1] : "demo";
    const int last = set == "seeds" ? 500009 : 500003;
    const int runSeeds = set == "seeds" ? 3 : 2;
    std::vector<RunKey> runs;
    std::vector<WorldState> layouts;
    for (int seed = 500000; seed <= last; ++seed) {
        const GeneratedLayout g = generateLayout(static_cast<std::uint64_t>(seed));
        std::cout << "layout " << seed << " attempts " << g.attempts << " shortest " << g.shortest << "\n";
        for (const std::string& row : renderWorld(g.world)) std::cout << row << "\n";
        for (int r = 1; r <= runSeeds; ++r) {
            runs.push_back(RunKey{static_cast<std::uint64_t>(seed), r});
            layouts.push_back(g.world);
        }
    }
    for (const std::vector<FaultEvent>& run : generateFaultEvents(runs, layouts, 4242)) {
        for (const FaultEvent& e : run) std::cout << faultEventText(e) << "\n";
    }
    return 0;
}
