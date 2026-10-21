// Golden-value tool for the Stage 2 rebuild (ai-hologram-v2 plan STAGE_02).
// Built ONLY against the ORIGINAL sources of IWatchSheep/ai-hologram at commit
// 7587f38: src/world (in ../../stage1/orig_trace/original_src) and src/model,
// src/compare, src/demo/prediction_demo (in ./original_src). Reference material,
// not project code (tools/reference/ is exempt from R1).
//
// Modes:
//   predict <cases>   blocks "case <n> action <Action>" + 10 belief rows + 10 true rows;
//                     prints the block header with " differences <k>", the 20 rows,
//                     "result <predicted> <observed> <match|MISMATCH>", then the k
//                     describeDifferences lines.
//   demo              prints the original runPredictionDemos output.
#include "compare/difference_text.h"
#include "compare/state_difference.h"
#include "demo/prediction_demo.h"
#include "model/belief.h"
#include "model/budget_counter.h"
#include "model/world_model.h"
#include "world/environment.h"
#include "world/world_text.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace ai_hologram;

static Action actionFrom(const std::string& name) {
    if (name == "North") return Action::North;
    if (name == "East") return Action::East;
    if (name == "South") return Action::South;
    return Action::West;
}

static int predictMode(const char* path) {
    std::ifstream in(path);
    std::string header;
    while (std::getline(in, header)) {
        if (header.empty()) continue;
        std::istringstream ss(header);
        std::string caseWord, actionWord, actionName;
        int n;
        ss >> caseWord >> n >> actionWord >> actionName;
        std::vector<std::string> beliefRows, trueRows;
        for (int i = 0; i < 10; ++i) { std::string r; std::getline(in, r); beliefRows.push_back(r); }
        for (int i = 0; i < 10; ++i) { std::string r; std::getline(in, r); trueRows.push_back(r); }
        ParseResult b = parseWorld(beliefRows);
        ParseResult t = parseWorld(trueRows);
        if (!b.world || !t.world) { std::cerr << "case " << n << " invalid\n"; return 2; }
        const WorldModel model;
        BudgetCounter budget(1);
        Environment environment(*t.world);
        const Action action = actionFrom(actionName);
        const std::optional<Prediction> prediction = model.predict(Belief{*b.world}, action, budget);
        const ActionResult observed = environment.step(action);
        const PredictionComparison c = comparePrediction(*prediction, observed);
        const std::vector<std::string> lines = describeDifferences(c.state);
        std::cout << header << " differences " << lines.size() << "\n";
        for (auto& r : beliefRows) std::cout << r << "\n";
        for (auto& r : trueRows) std::cout << r << "\n";
        std::cout << "result " << outcomeName(c.predictedOutcome) << " " << outcomeName(c.observedOutcome)
                  << " " << (matches(c) ? "match" : "MISMATCH") << "\n";
        for (auto& l : lines) std::cout << l << "\n";
    }
    return 0;
}

int main(int argc, char** argv) {
    std::string mode = argc > 1 ? argv[1] : "";
    if (mode == "predict" && argc > 2) return predictMode(argv[2]);
    if (mode == "demo") { runPredictionDemos(std::cout); return 0; }
    std::cerr << "usage: orig_prediction_trace predict <file> | demo\n";
    return 1;
}
