// Purpose: Runs the two C1 demonstrations of plan 3.12:
// demo 1 with a correct belief, and demo 2 with a
// hand-written belief that wrongly has no wall at (4, 3).
// Introduced in: Stage 3f (rebuild)
// Architecture: Both demonstrations use the shared demo
// layout (demoLayout, rule R4), the illustrative settings
// of plan 3.12, and the real run loop. Demo 2's starting
// belief is hand-written in this file: the true world is
// the demo layout plus a wall at (4, 3), and the belief is
// the demo layout without it. That is a hand-written wrong
// belief, labelled as such; it is not fault injection.

#include "demo/agent_demo.h"

#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

#include "agent/agent_types.h"
#include "agent/observation.h"
#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "demo/scripted_demo.h"
#include "world/position.h"
#include "world/world_state.h"
#include "world/world_text.h"

namespace aih {
namespace {

// Writes every line of a finished run to out.
void printRunLines(std::ostream& out, const std::vector<std::string>& lines) {
    for (const std::string& line : lines) {
        out << line << '\n';
    }
}

// The first observation of a run in a true world.
Observation firstObservation(const WorldState& trueWorld) {
    Observation first;
    first.decisionStep = 1;
    first.state = trueWorld;
    return first;
}

// Returns a copy of world with the wall at cell changed.
WorldState withWall(const WorldState& world, Position cell, bool blocked) {
    WorldState copy = world;
    copy.walls[static_cast<std::size_t>(cell.y)]
              [static_cast<std::size_t>(cell.x)] = blocked;
    return copy;
}

}  // namespace

void runAgentDemos(std::ostream& out) {
    const ParseResult parsed = parseWorld(demoLayout());
    if (!parsed.world.has_value()) {
        out << "Agent demo layout is invalid.\n";
        return;
    }
    const AgentSettings settings{200, 5, 100, 3};
    const WorldState demo = parsed.world.value();
    out << "\nSingle-agent demonstrations use illustrative settings: "
           "budget 200 per step,\n";
    out << "safe-stop limit 5, step limit 100, active-test delay 3 "
           "(not the experiment's values).\n";
    out << "\nC1 demo 1: correct belief\n";
    SingleAgent demo1Agent(firstObservation(demo), settings);
    const RunResult demo1 = runSingleAgent(demo, demo1Agent, settings);
    printRunLines(out, demo1.lines);
    const WorldState trueWorld = withWall(demo, Position{4, 3}, true);
    out << "\nC1 demo 2: belief wrongly has no wall at (4, 3)\n";
    SingleAgent demo2Agent(firstObservation(trueWorld), settings, demo);
    const RunResult demo2 = runSingleAgent(trueWorld, demo2Agent, settings);
    printRunLines(out, demo2.lines);
    out << "Note: demo 2's wrong belief is hand-written; it is not "
           "fault injection.\n";
}

}  // namespace aih
