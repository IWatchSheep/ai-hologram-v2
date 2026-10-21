// Purpose: Tests for C1's full runs (C1-1, C1-10, C1-2,
// C1-5, C1-9, C1-12a, C1-12b, plan Section 6.5).
// Introduced in: Stage 3e (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld. Expected
// lines come from checks.py (check_single_agent), shared
// for the two demos through expected_demo_lines (R4), and
// each description starts with its check id.

#include <string>
#include <vector>

#include "agent/agent_types.h"
#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
#include "expected_demo_lines.h"
#include "model/world_model.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

// The push-turn model of checks.py (pushturn East North):
// an East push puts the agent on the object's cell and the
// object one cell North, or blocks when that cell is a wall.
ActionResult pushTurnNorth(const WorldState& state, Action action) {
    const ActionResult standard = applyAction(state, action);
    if (action != Action::East) {
        return standard;
    }
    if (standard.outcome != MoveOutcome::Pushed) {
        return standard;
    }
    const Position turned = moved(state.object, Action::North);
    if (isWall(state, turned)) {
        return ActionResult{state, MoveOutcome::Blocked};
    }
    ActionResult result;
    result.state = state;
    result.state.agent = state.object;
    result.state.object = turned;
    result.outcome = MoveOutcome::Pushed;
    return result;
}

void checkC1Demo1(TestCounter& counter) {
    const WorldState demo = demoWorld();
    SingleAgent agent(makeObservation(1, demo), AgentSettings{});
    const RunResult run = runSingleAgent(demo, agent, AgentSettings{});
    check(counter,
          isValidWorld(demo) &&
              linesEqual(c1Demo1Lines(), run.lines, *counter.out),
          "C1-1: demo 1 (D, B 200, S_max 5, T_max 100, T_v 3): run "
          "lines exactly C1_1 (19 evaluations)");
    bool ok = run.lines.size() == 9;
    if (ok) {
        ok = run.lines[4] ==
             "Step 4: East | predicted Pushed, observed Pushed | match";
    }
    check(counter, ok && agent.state.decisionStep == 4,
          "C1-10: 5th line is the final check's Step 4 match; "
          "decisionStep 4");
}

void checkC1Demo2(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    SingleAgent agent(makeObservation(1, d2), AgentSettings{}, demo);
    const RunResult run = runSingleAgent(d2, agent, AgentSettings{});
    bool ok = linesEqual(c1Demo2Lines(), run.lines, *counter.out);
    ok = ok && run.lines.size() > 8;
    if (run.lines.size() > 8) {
        ok = ok &&
             run.lines[8] == "Plan: active test of correction 1 (2 actions)";
    }
    check(counter, isValidWorld(d2) && ok,
          "C1-2: demo 2: run lines exactly C1_2 (414 evaluations), "
          "active-test line at index 8");
}

void checkC1Lt(TestCounter& counter) {
    AgentSettings settings;
    settings.safeStopLimit = 2;
    const WorldState lt = ltWorld();
    SingleAgent agent(makeObservation(1, lt), settings);
    const RunResult run = runSingleAgent(lt, agent, settings);
    const std::vector<std::string> expected = {
        "Step 1: safe stop (no safe action)",
        "Step 2: safe stop (no safe action)",
        "Result: Stopped -- unresolved after 0 world steps, 2 safe stops",
        "Mismatches: 0",
        "Corrections: 0 (0 verified)",
        "Model evaluations used: 8"};
    bool ok = linesEqual(expected, run.lines, *counter.out);
    ok = ok && agent.state.decisionStep == 2;
    ok = ok && agent.state.history.size() == 2;
    if (agent.state.history.size() == 2) {
        ok = ok && !agent.state.history[1].lastAction.has_value();
    }
    check(counter, isValidWorld(lt) && ok,
          "C1-5: LT, S_max 2: exactly C1_5; decisionStep 2; second "
          "observation has no last action");
}

void checkC1PushTurn(TestCounter& counter) {
    AgentSettings settings;
    settings.maxWorldSteps = 6;
    const WorldState demo = demoWorld();
    SingleAgent agent(makeObservation(1, demo), settings, std::nullopt,
                      ActionModel(pushTurnNorth));
    const RunResult run = runSingleAgent(demo, agent, settings);
    const std::vector<std::string> expected = {
        "Plan: fallback toward the best state found (1 actions)",
        "Step 1: East | predicted Moved, observed Moved | match",
        "Plan: fallback toward the best state found (2 actions)",
        "Step 2: North | predicted Moved, observed Moved | match",
        "Step 3: East | predicted Moved, observed Moved | match",
        "Plan: fallback toward the best state found (2 actions)",
        "Step 4: East | predicted Moved, observed Moved | match",
        "Step 5: South | predicted Moved, observed Moved | match",
        "Plan: fallback toward the best state found (2 actions)",
        "Step 6: North | predicted Moved, observed Moved | match",
        "Result: Step limit after 6 world steps, 0 safe stops",
        "Mismatches: 0",
        "Corrections: 0 (0 verified)",
        "Model evaluations used: 798"};
    check(counter,
          isValidWorld(demo) && linesEqual(expected, run.lines, *counter.out),
          "C1-9: push-turn model, T_max 6: exactly C1_9 (798 evaluations)");
}

void checkC1GoalAtStart(TestCounter& counter) {
    const WorldState lg = lgWorld();
    SingleAgent agent(makeObservation(1, lg), AgentSettings{});
    const RunResult run = runSingleAgent(lg, agent, AgentSettings{});
    bool ok = run.outcome == "Success" && run.worldSteps == 0;
    ok = ok && agent.totalEvaluations() == 0;
    ok = ok && agent.state.decisionStep == 0;
    check(counter, isValidWorld(lg) && ok,
          "C1-12a: LG: Success after 0 world steps, 0 evaluations, "
          "decide never called");
    AgentSettings settings;
    settings.safeStopLimit = 2;
    const WorldState demo = demoWorld();
    const WorldState wrong = withObjectAt(demo, Position{8, 3});
    SingleAgent believer(makeObservation(1, demo), settings, wrong);
    const RunResult stopped = runSingleAgent(demo, believer, settings);
    const std::vector<std::string> expected = {
        "Step 1: safe stop (belief shows goal reached)",
        "Step 2: safe stop (belief shows goal reached)",
        "Result: Stopped -- unresolved after 0 world steps, 2 safe stops",
        "Mismatches: 0",
        "Corrections: 0 (0 verified)",
        "Model evaluations used: 0"};
    check(counter,
          isValidWorld(wrong) &&
              linesEqual(expected, stopped.lines, *counter.out),
          "C1-12b: belief object on destination, S_max 2: exactly C1_12B");
}

}  // namespace

void runSingleAgentTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Single agent ---\n";
    }
    checkC1Demo1(counter);
    checkC1Demo2(counter);
    checkC1Lt(counter);
    checkC1PushTurn(counter);
    checkC1GoalAtStart(counter);
}

}  // namespace aih
