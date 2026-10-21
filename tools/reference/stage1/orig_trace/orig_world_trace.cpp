// Golden-value tool for the Stage 1 rebuild (ai-hologram-v2 plan STAGE_01).
// Built ONLY against the ORIGINAL world sources of IWatchSheep/ai-hologram at
// commit 7587f38 (tag stage-3; src/world is unchanged from stage-1 to 89ea9b6).
// It is reference material, not project code (tools/reference/ is exempt from R1).
//
// Modes:
//   rules  <cases file>   one case per line: layout-id ax ay ox oy action
//                         prints the same line + " -> outcome ax ay ox oy goal"
//   parse  <cases file>   blocks: "case <n> lines <k>" followed by k raw lines;
//                         prints "case <n>: OK ax ay ox oy dx dy" or "case <n>: ERROR <message>"
//   demo                  prints the original runScriptedDemo output
// Layouts for rules mode are read from the cases file header:
//   "layout <id>" followed by 10 lines (parsed with the original parseWorld).
#include "world/rules.h"
#include "world/world_text.h"
#include "demo/scripted_demo.h"

#include <fstream>
#include <iostream>
#include <map>
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

static int rulesMode(const char* path) {
    std::ifstream in(path);
    std::string line;
    std::map<std::string, WorldState> layouts;
    while (std::getline(in, line)) {
        if (line.rfind("layout ", 0) == 0) {
            std::string id = line.substr(7);
            std::vector<std::string> rows;
            for (int i = 0; i < 10; ++i) { std::string r; std::getline(in, r); rows.push_back(r); }
            ParseResult p = parseWorld(rows);
            if (!p.world) { std::cerr << "bad layout " << id << ": " << p.error << "\n"; return 2; }
            layouts[id] = *p.world;
            std::cout << line << "\n";
            for (auto& r : rows) std::cout << r << "\n";
            continue;
        }
        if (line.empty()) continue;
        std::istringstream ss(line);
        std::string id, act; int ax, ay, ox, oy;
        ss >> id >> ax >> ay >> ox >> oy >> act;
        WorldState s = layouts.at(id);
        s.agent = {ax, ay};
        s.object = {ox, oy};
        ActionResult r = applyAction(s, actionFrom(act));
        std::cout << line << " -> " << outcomeName(r.outcome) << " " << r.state.agent.x << " "
                  << r.state.agent.y << " " << r.state.object.x << " " << r.state.object.y << " "
                  << (isGoalReached(r.state) ? 1 : 0) << "\n";
    }
    return 0;
}

static int parseMode(const char* path) {
    std::ifstream in(path);
    std::string header;
    while (std::getline(in, header)) {
        if (header.empty()) continue;
        std::istringstream ss(header);
        std::string word, linesWord; int n, k;
        ss >> word >> n >> linesWord >> k;
        std::vector<std::string> rows;
        for (int i = 0; i < k; ++i) { std::string r; std::getline(in, r); rows.push_back(r); }
        ParseResult p = parseWorld(rows);
        if (p.world) {
            const WorldState& w = *p.world;
            bool round = renderWorld(w) == rows;
            std::cout << "case " << n << ": OK " << w.agent.x << " " << w.agent.y << " " << w.object.x
                      << " " << w.object.y << " " << w.destination.x << " " << w.destination.y
                      << (round ? "" : " ROUNDTRIP-DIFFERS") << "\n";
        } else {
            std::cout << "case " << n << ": ERROR " << p.error << "\n";
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    std::string mode = argc > 1 ? argv[1] : "";
    if (mode == "rules" && argc > 2) return rulesMode(argv[2]);
    if (mode == "parse" && argc > 2) return parseMode(argv[2]);
    if (mode == "demo") { runScriptedDemo(std::cout); return 0; }
    std::cerr << "usage: orig_world_trace rules|parse <file> | demo\n";
    return 1;
}
