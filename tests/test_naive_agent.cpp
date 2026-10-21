// Purpose: Tests for C0, the naive agent (C0-1 to C0-5 and
// OB7, plan Section 6.6).
// Introduced in: Stage 3f (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld. The
// expected actions, counts and run lines are checks.py's
// C0_4, C0_5 and C0_5_FIRST_TEN. The hand-run loops decide,
// step an Environment and build the next observation from
// it, exactly as the run loop does.

#include <array>
#include <string>
#include <vector>

#include "agent/agent_types.h"
#include "agent/naive_agent.h"
#include "agent/observation.h"
#include "agent/run_loop.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/environment.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

// One hook report of OB7, as (step, plan size, evaluations
// used) at the moment the hook is called.
using HookSeen = std::array<int, 3>;

void checkC01(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    const Observation first = makeObservation(1, d2);
    NaiveAgent agentA(first, AgentSettings{}, demo);
    NaiveAgent agentB(first, AgentSettings{}, demo);
    Environment environment(d2);
    Observation current = first;
    bool same = true;
    for (int i = 0; i < 20; i = i + 1) {
        const Decision decisionA = agentA.decide(current);
        const Decision decisionB = agentB.decide(first);
        same = same && decisionA.action == decisionB.action;
        const ActionResult stepped = environment.step(decisionA.action);
        current =
            makeObservation(agentA.state.decisionStep + 1, environment.state());
        current.lastAction = decisionA.action;
        current.lastOutcome = stepped.outcome;
    }
    same = same && agentA.totalEvaluations() == agentB.totalEvaluations();
    check(counter, isValidWorld(d2) && same,
          "C0-1: D2, belief D: true observations and the first "
          "observation repeated give identical actions and "
          "evaluations over 20 decisions");
}

void checkC02(TestCounter& counter) {
    const WorldState demo = demoWorld();
    NaiveAgent agent(makeObservation(1, demo), AgentSettings{});
    Environment environment(demo);
    Observation current = makeObservation(1, demo);
    std::vector<Action> taken;
    std::vector<int> used;
    for (int i = 0; i < 4; i = i + 1) {
        const Decision decision = agent.decide(current);
        taken.push_back(decision.action);
        used.push_back(agent.state.predictor.usedThisStep());
        const ActionResult stepped = environment.step(decision.action);
        current =
            makeObservation(agent.state.decisionStep + 1, environment.state());
        current.lastAction = decision.action;
        current.lastOutcome = stepped.outcome;
    }
    const std::vector<Action> expected = {Action::East, Action::East,
                                          Action::East, Action::East};
    bool ok = taken == expected && used == std::vector<int>{16, 0, 0, 0};
    ok = ok && agent.searchCount() == 1;
    check(counter, isValidWorld(demo) && ok,
          "C0-2: D: East x4; usedThisStep 16, 0, 0, 0; one search");
}

void checkC03(TestCounter& counter) {
    const WorldState lta = ltaWorld();
    NaiveAgent agentA(makeObservation(1, lta), AgentSettings{});
    const Decision firstA = agentA.decide(makeObservation(1, lta));
    bool ok = firstA.action == Action::East;
    ok = ok && agentA.state.predictor.usedThisStep() == 4;
    ok = ok && agentA.searchCount() == 1;
    ok = ok && agentA.state.belief.agent == Position{6, 1};
    check(counter, isValidWorld(lta) && ok,
          "C0-3a: LTa: first decision East, 4 evaluations, one "
          "search; belief agent at (6, 1)");
    const WorldState boxed = boxedWorld();
    NaiveAgent agentB(makeObservation(1, boxed), AgentSettings{});
    const Decision firstB = agentB.decide(makeObservation(1, boxed));
    bool okBoxed = firstB.action == Action::North;
    okBoxed = okBoxed && agentB.state.predictor.usedThisStep() == 4;
    okBoxed = okBoxed && agentB.state.belief == boxed;
    check(counter, isValidWorld(boxed) && okBoxed,
          "C0-3b: BOXED: first decision North, 4 evaluations; "
          "belief unchanged");
}

void checkC04(TestCounter& counter) {
    const WorldState demo = demoWorld();
    NaiveAgent agent(makeObservation(1, demo), AgentSettings{});
    const RunResult run = runSingleAgent(demo, agent, AgentSettings{});
    const std::vector<std::string> expected = {
        "Result: Success after 4 world steps, 0 safe stops", "Mismatches: 0",
        "Corrections: 0 (0 verified)", "Model evaluations used: 16"};
    bool ok = linesEqual(expected, run.lines, *counter.out);
    ok = ok && run.outcome == "Success" && run.worldSteps == 4;
    check(counter, isValidWorld(demo) && ok,
          "C0-4: D: run lines exactly C0_4 (16 evaluations)");
}

void checkC05(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    NaiveAgent agent(makeObservation(1, d2), AgentSettings{}, demo);
    std::vector<Action> taken;
    NaiveRunWatchers watchers;
    watchers.afterDecision = [&taken](const NaiveAgent&, const Observation&,
                                      const Decision& decision) {
        taken.push_back(decision.action);
    };
    const RunResult run = runSingleAgent(d2, agent, AgentSettings{}, watchers);
    const std::vector<std::string> expectedLines = {
        "Result: Step limit after 100 world steps, 0 safe stops",
        "Mismatches: 0", "Corrections: 0 (0 verified)",
        "Model evaluations used: 207"};
    const std::vector<Action> expectedFirst = {
        Action::East,  Action::East,  Action::East, Action::East,
        Action::North, Action::North, Action::East, Action::South,
        Action::North, Action::South};
    bool ok = linesEqual(expectedLines, run.lines, *counter.out);
    ok = ok && agent.searchCount() == 97 && taken.size() == 100;
    if (taken.size() >= expectedFirst.size()) {
        const std::vector<Action> firstTen(taken.begin(), taken.begin() + 10);
        ok = ok && firstTen == expectedFirst;
    }
    check(counter, isValidWorld(d2) && ok,
          "C0-5: D2, belief D: exactly C0_5 (Step limit after "
          "100, 207 evaluations), 97 searches, first ten actions");
}

void checkOb7(TestCounter& counter) {
    const WorldState demo = demoWorld();
    NaiveAgent agent(makeObservation(1, demo), AgentSettings{});
    std::vector<HookSeen> seen;
    agent.state.activationHook = [&seen, &agent](int step) {
        const int planSize = static_cast<int>(agent.state.plan.size());
        const int used = agent.state.predictor.usedThisStep();
        seen.push_back(HookSeen{step, planSize, used});
    };
    runSingleAgent(demo, agent, AgentSettings{});
    const std::vector<HookSeen> expected = {
        {1, 0, 0}, {2, 3, 0}, {3, 2, 0}, {4, 1, 0}};
    check(counter, isValidWorld(demo) && seen == expected,
          "OB7: C0 activation hook at the start of each step: "
          "(step, plan size, used) (1, 0, 0), (2, 3, 0), "
          "(3, 2, 0), (4, 1, 0)");
}

}  // namespace

void runNaiveAgentTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Naive agent ---\n";
    }
    checkC01(counter);
    checkC02(counter);
    checkC03(counter);
    checkC04(counter);
    checkC05(counter);
    checkOb7(counter);
}

}  // namespace aih
