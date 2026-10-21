// Purpose: Defines the shared test worlds and helpers for
// the agent tests.
// Introduced in: Stage 3a (rebuild)
// Architecture: The layouts are written out in full, exactly
// as in checks.py, so a reader can compare them line by
// line. Each is parsed once per call, so no test can change
// another test's world.

#include "agent_test_support.h"

#include <algorithm>
#include <cstddef>
#include <ostream>
#include <string>
#include <vector>

#include "agent/dead_squares.h"
#include "agent/heuristic.h"
#include "agent/planner.h"
#include "agent/step_predictor.h"
#include "demo/scripted_demo.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"
#include "world/world_text.h"

namespace aih {
namespace {

// Parses a layout into a world, or an empty state if the
// layout failed to parse. Every layout here is valid, as
// the plan requires of every fixture.
WorldState worldFromLayout(const std::vector<std::string>& layout) {
    const ParseResult result = parseWorld(layout);
    if (result.world.has_value()) {
        return result.world.value();
    }
    return WorldState{};
}

}  // namespace

WorldState demoWorld() {
    return worldFromLayout(demoLayout());
}

WorldState lpWorld() {
    return worldFromLayout(
        {"##########", "#....D##.#", "#.O......#", "#....A#..#", "#.#......#",
         "#.....##.#", "#........#", "#........#", "#........#", "##########"});
}

WorldState l2World() {
    return worldFromLayout(
        {"##########", "#....#...#", "#.O..#.A.#", "#....#...#", "#D...#...#",
         "######...#", "#........#", "#........#", "#........#", "##########"});
}

WorldState ltWorld() {
    return worldFromLayout(
        {"##########", "#O.......#", "#........#", "#........#", "#....A...#",
         "#........#", "#........#", "#.......D#", "#........#", "##########"});
}

WorldState lgWorld() {
    return worldFromLayout(
        {"##########", "#........#", "#........#", "#..A....*#", "#........#",
         "#........#", "#...##...#", "#........#", "#........#", "##########"});
}

WorldState ltaWorld() {
    return worldFromLayout(
        {"##########", "#O...A...#", "#........#", "#........#", "#........#",
         "#........#", "#........#", "#.......D#", "#........#", "##########"});
}

WorldState boxedWorld() {
    return worldFromLayout(
        {"##########", "#.#......#", "##A#.....#", "#.#..O..D#", "#........#",
         "#........#", "#...##...#", "#........#", "#........#", "##########"});
}

WorldState smallWorld() {
    return worldFromLayout(
        {"##########", "#........#", "#........#", "#..AO.D..#", "#........#",
         "#........#", "#........#", "#........#", "#........#", "##########"});
}

WorldState withAgentAt(const WorldState& world, Position agent) {
    WorldState copy = world;
    copy.agent = agent;
    return copy;
}

WorldState withObjectAt(const WorldState& world, Position object) {
    WorldState copy = world;
    copy.object = object;
    return copy;
}

WorldState withWallAt(const WorldState& world, Position cell, bool blocked) {
    WorldState copy = world;
    const std::size_t x = static_cast<std::size_t>(cell.x);
    const std::size_t y = static_cast<std::size_t>(cell.y);
    copy.walls[y][x] = blocked;
    return copy;
}

bool linesEqual(const std::vector<std::string>& expected,
                const std::vector<std::string>& actual, std::ostream& out) {
    const std::size_t common =
        expected.size() < actual.size() ? expected.size() : actual.size();
    for (std::size_t i = 0; i < common; i = i + 1) {
        if (expected[i] != actual[i]) {
            out << "first differing line " << (i + 1) << ": expected ["
                << expected[i] << "] actual [" << actual[i] << "]\n";
            return false;
        }
    }
    if (expected.size() != actual.size()) {
        out << "line counts differ: expected " << expected.size() << " actual "
            << actual.size() << "\n";
        return false;
    }
    return true;
}

SearchRun runMainSearchCounting(const WorldState& root, int budget, int reserve,
                                int& modelCalls) {
    FreeComputationCounts counts;
    SearchRun run;
    run.map = computeDeadSquares(root, {}, {}, counts);
    const DeadSquareMap& map = run.map;
    const ActionModel counting = [&modelCalls](const WorldState& state,
                                               Action action) {
        modelCalls = modelCalls + 1;
        return applyAction(state, action);
    };
    StepPredictor predictor(WorldModel(counting), budget);
    predictor.startStep();
    auto goal = [](const WorldState& state) {
        return state.object == state.destination;
    };
    auto estimate = [&map, &counts](const WorldState& state) {
        return heuristic(state, map, counts);
    };
    run.result = search(root, map, goal, estimate, predictor, reserve);
    run.usedThisStep = predictor.usedThisStep();
    run.remaining = predictor.remaining();
    return run;
}

SearchRun runMainSearch(const WorldState& root, int budget, int reserve) {
    int modelCalls = 0;
    return runMainSearchCounting(root, budget, reserve, modelCalls);
}

SearchRun runActiveWalk(const WorldState& root,
                        const std::vector<Position>& targets) {
    FreeComputationCounts counts;
    SearchRun run;
    run.map = computeDeadSquares(root, {}, {}, counts);
    const DeadSquareMap& map = run.map;
    StepPredictor predictor(WorldModel(), 1000000);
    predictor.startStep();
    auto goal = [&targets](const WorldState& state) {
        // True when the agent stands on any target cell.
        return std::find(targets.begin(), targets.end(), state.agent) !=
               targets.end();
    };
    auto estimate = [&map, &targets](const WorldState& state) {
        if (readDeadStatus(map, state.object)) {
            return kInfiniteHeuristic;
        }
        int best = kInfiniteHeuristic;
        for (const Position target : targets) {
            best = std::min(best, manhattan(state.agent, target));
        }
        return best;
    };
    run.result = search(root, map, goal, estimate, predictor, 1);
    run.usedThisStep = predictor.usedThisStep();
    run.remaining = predictor.remaining();
    return run;
}

std::vector<Action> planActions(const std::vector<PlanStep>& plan) {
    std::vector<Action> actions;
    for (const PlanStep& step : plan) {
        actions.push_back(step.action);
    }
    return actions;
}
}  // namespace aih
