// Purpose: Tests for C1 at tight budgets (C1-11a, C1-11b,
// VF6, SP7-B1 to SP7-B11, plan Section 6.5).
// Introduced in: Stage 3e (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld. Agents
// are seeded through agent_test_fixtures, exactly as
// checks.py seeds them. Expected values come from
// checks.py (check_single_agent): C1_11, VF6_LINES and
// SP7_EXPECTED.

#include <string>
#include <vector>

#include "agent/agent_types.h"
#include "agent/observation.h"
#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

void checkC1Deferred(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    AgentSettings settings;
    settings.budgetPerStep = 1;
    SingleAgent agent(makeObservation(1, d2), settings, demo);
    seedCommitted(agent, Action::East);
    const ActionResult blocked = applyAction(d2, Action::East);
    Observation second = makeObservation(2, blocked.state);
    second.lastAction = Action::East;
    second.lastOutcome = blocked.outcome;
    const Decision first = agent.decide(second);
    const Decision next = agent.decide(makeObservation(3, blocked.state));
    bool ok = !first.isAction && !next.isAction;
    ok = ok && first.stopReason == "budget too small for verification";
    ok = ok && next.stopReason == "budget too small for verification";
    const std::string deferred =
        "  - investigation: budget too small for verification; retried "
        "next step";
    bool found = false;
    for (const std::string& line : agent.logLines()) {
        if (line == deferred) {
            found = true;
        }
    }
    ok = ok && found && agent.mismatchCount() == 1;
    ok = ok && agent.correctionCount() == 0;
    check(counter, isValidWorld(d2) && ok,
          "C1-11a: deferred: D2, belief D, B 1, committed East: two "
          "budget-too-small stops, deferred line, 1 mismatch, 0 "
          "corrections");
}

void checkC1NoBudget(TestCounter& counter) {
    const WorldState demo = demoWorld();
    AgentSettings settings;
    settings.budgetPerStep = 0;
    SingleAgent agent(makeObservation(1, demo), settings);
    seedPlan(agent, Action::East);
    const Decision stopped = agent.decide(makeObservation(1, demo));
    bool ok = !stopped.isAction;
    ok = ok && stopped.stopReason == "no budget for a committed prediction";
    check(counter, isValidWorld(demo) && ok,
          "C1-11b: B 0 with a plan in place: safe stop no budget for a "
          "committed prediction");
}

void checkVf6(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState belief = withWallAt(demo, Position{4, 3}, true);
    SingleAgent agent(makeObservation(1, demo), AgentSettings{}, belief);
    seedWallCorrections(agent, {Position{4, 3}}, true, false);
    const RunResult run = runSingleAgent(demo, agent, AgentSettings{});
    const std::vector<std::string> expected = {
        "Plan: fallback toward the best state found (10 actions)",
        "Step 1: North | predicted Moved, observed Moved | match",
        "Step 2: East | predicted Moved, observed Moved | match",
        "Plan: active test of correction 1 (1 actions)",
        "Step 3: South | predicted Blocked, observed Moved | MISMATCH",
        "  - correction 1: REJECTED",
        "  - investigation: wall at (4, 3) believed blocked, recorded "
        "free (source S2)",
        "  - correction 2: wall at (4, 3) set to free: CONSISTENT",
        "Plan: search found a plan of 3 actions",
        "Step 4: East | predicted Pushed, observed Pushed | match",
        "Step 5: East | predicted Pushed, observed Pushed | match",
        "Step 6: East | predicted Pushed, observed Pushed | match",
        "Result: Success after 6 world steps, 0 safe stops",
        "Mismatches: 1",
        "Corrections: 2 (0 verified)",
        "Model evaluations used: 216"};
    check(counter,
          isValidWorld(belief) && linesEqual(expected, run.lines, *counter.out),
          "VF6: belief D + wall (4, 3), true D, seeded wrong wall "
          "correction: exactly VF6_LINES (216 evaluations)");
}

struct Sp7Expectation {
    const char* outcome;
    int worldSteps;
    int safeStops;
};

// At B <= 4 the run must stop three times with
// "budget too small to plan" and take no world step.
bool smallBudgetOk(int budget, const RunResult& run,
                   const std::vector<std::string>& reasons) {
    if (budget > 4) {
        return true;
    }
    bool ok = run.worldSteps == 0 && reasons.size() == 3;
    for (const std::string& reason : reasons) {
        ok = ok && reason == "budget too small to plan";
    }
    return ok;
}

void checkSp7One(TestCounter& counter, int budget,
                 const Sp7Expectation& expected) {
    const WorldState demo = demoWorld();
    AgentSettings settings;
    settings.budgetPerStep = budget;
    settings.safeStopLimit = 3;
    settings.maxWorldSteps = 30;
    SingleAgent agent(makeObservation(1, demo), settings);
    seedWallCorrections(agent, {Position{4, 3}, Position{4, 2}, Position{3, 4}},
                        false, true);
    int maxUsed = 0;
    bool committed = true;
    std::vector<std::string> reasons;
    RunWatchers watchers;
    watchers.afterDecision = [&](const SingleAgent& watched, const Observation&,
                                 const Decision& d) {
        if (d.isAction && !watched.state.committed.has) {
            committed = false;
        }
        if (!d.isAction) {
            reasons.push_back(d.stopReason);
        }
        const int used = watched.state.predictor.usedThisStep();
        maxUsed = used > maxUsed ? used : maxUsed;
    };
    const RunResult run = runSingleAgent(demo, agent, settings, watchers);
    bool ok = run.outcome == expected.outcome;
    ok = ok && run.worldSteps == expected.worldSteps;
    ok = ok && run.safeStops == expected.safeStops;
    ok = ok && smallBudgetOk(budget, run, reasons);
    ok = ok && maxUsed <= budget && committed;
    const std::string id = "SP7-B" + std::to_string(budget);
    check(counter, isValidWorld(demo) && ok,
          id + ": D with three seeded wall corrections, S_max 3, "
               "T_max 30: outcome, world steps and safe stops of "
               "SP7_EXPECTED; no step over B; every action committed");
}

void checkSp7(TestCounter& counter) {
    const Sp7Expectation expected[11] = {{"Stopped -- unresolved", 0, 3},
                                         {"Stopped -- unresolved", 0, 3},
                                         {"Stopped -- unresolved", 0, 3},
                                         {"Stopped -- unresolved", 0, 3},
                                         {"Stopped -- unresolved", 2, 3},
                                         {"Success", 4, 0},
                                         {"Success", 4, 0},
                                         {"Step limit", 30, 0},
                                         {"Step limit", 30, 0},
                                         {"Step limit", 30, 0},
                                         {"Success", 12, 0}};
    for (int budget = 1; budget <= 11; budget = budget + 1) {
        checkSp7One(counter, budget, expected[budget - 1]);
    }
}

}  // namespace

void runSingleAgentBudgetTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Single agent (budget) ---\n";
    }
    checkC1Deferred(counter);
    checkC1NoBudget(counter);
    checkVf6(counter);
    checkSp7(counter);
}

}  // namespace aih
