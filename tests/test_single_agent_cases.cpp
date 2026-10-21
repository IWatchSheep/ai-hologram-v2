// Purpose: More C1 tests (C1-3, C1-4, C1-6a, C1-6b, VF5b,
// C1-6c-1, C1-6c-2, C1-7, plan Section 6.5).
// Introduced in: Stage 3e (rebuild)
// Architecture: Fixtures are valid worlds; runs use
// the Watch of checks.py. Expected: checks.py.

#include <algorithm>
#include <vector>

#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
#include "test_suites.h"
#include "test_support.h"

namespace aih {
namespace {

// The pending indices of an agent's V1 work, or none.
std::vector<int> pendingOf(const SingleAgent& agent) {
    if (agent.state.pendingV1.has_value()) {
        return agent.state.pendingV1->pendingIndices;
    }
    return {};
}

// True when two observations are the same in every field.
bool sameObservation(const Observation& first, const Observation& second) {
    bool same = first.decisionStep == second.decisionStep;
    same = same && first.state == second.state;
    same = same && first.lastAction == second.lastAction;
    return same && first.lastOutcome == second.lastOutcome;
}

// Runs one correct-belief world (checks.py, C1-3).
bool runBeliefsTrue(const WorldState& truth) {
    SingleAgent agent(makeObservation(1, truth), AgentSettings{});
    StepWatch watch;
    const RunResult run =
        runSingleAgent(truth, agent, AgentSettings{}, watch.watchers());
    bool ok = run.outcome == "Success" && agent.mismatchCount() == 0;
    return ok && watch.beliefsTrue && watch.checks > 0;
}

void checkC1BeliefsAndHistory(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    bool ok = runBeliefsTrue(demo) && runBeliefsTrue(lpWorld());
    ok = ok && runBeliefsTrue(d2);
    check(counter, isValidWorld(d2) && ok,
          "C1-3: D, LP, D2 correct beliefs: Success, belief = observation, no "
          "mismatch");
    SingleAgent agent(makeObservation(1, demo), AgentSettings{});
    StepWatch watch;
    runSingleAgent(demo, agent, AgentSettings{}, watch.watchers());
    std::vector<Observation> seen = watch.delivered;
    seen.insert(seen.end(), watch.finals.begin(), watch.finals.end());
    bool historyOk = agent.state.history.size() == 5 && seen.size() == 5;
    if (historyOk) {
        historyOk =
            std::equal(agent.state.history.begin(), agent.state.history.end(),
                       seen.begin(), sameObservation);
    }
    check(
        counter, historyOk,
        "C1-4: history holds the 5 delivered observations unchanged, in order");
}

void checkC1Cautious(TestCounter& counter) {
    const WorldState demo = demoWorld();
    SingleAgent agent(makeObservation(1, demo), AgentSettings{});
    agent.state.predictor.startStep();
    const std::optional<PlanStep> pick = agent.chooseCautiousAction();
    bool ok = pick.has_value() && pick->action == Action::North;
    ok = ok && agent.state.predictor.usedThisStep() == 2;
    check(
        counter, isValidWorld(demo) && ok,
        "C1-6a: chooseCautiousAction() alone on D picks North, 2 evaluations");
    AgentSettings settings{2, 5, 100, 3};
    SingleAgent pending(makeObservation(1, demo), settings);
    seedModelV1(pending, 2);
    const Decision first = pending.decide(makeObservation(1, demo));
    bool okB = !first.isAction;
    okB = okB &&
          first.stopReason == "investigation unfinished, no cautious action";
    okB = okB && pendingOf(pending) == std::vector<int>{2};
    check(counter, okB,
          "C1-6b: B 2, V1 work East, 2 replays: safe stop, [2] left");
    const WorldState afterTwo = pending.state.history[2].state;
    pending.decide(makeObservation(2, afterTwo));
    bool okC = !pending.state.pendingV1.has_value();
    okC = okC && pending.correctionCount() == 1;
    okC = okC && pending.state.agentModel.isRestored(Action::East);
    const std::string line =
        "  - correction 1: action model for East restored to the standard "
        "rule: CONSISTENT";
    okC = okC && std::find(pending.logLines().begin(), pending.logLines().end(),
                           line) != pending.logLines().end();
    check(counter, okC,
          "VF5b: next decision finishes V1: 1 correction, East restored, line "
          "logged");
}

void checkC1SixC1(TestCounter& counter) {
    const WorldState demo = demoWorld();
    AgentSettings two{2, 2, 100, 3};
    SingleAgent stopped(makeObservation(1, demo), two);
    seedModelV1(stopped, 3);
    const RunResult run = runSingleAgent(demo, stopped, two);
    const std::vector<std::string> expected = {
        "Step 1: safe stop (investigation unfinished, no cautious action)",
        "Step 2: safe stop (investigation unfinished, no cautious action)",
        "Result: Stopped -- unresolved after 0 world steps, 2 safe stops",
        "Mismatches: 0",
        "Corrections: 0 (0 verified)",
        "Model evaluations used: 2"};
    bool ok = linesEqual(expected, run.lines, *counter.out);
    ok = ok && pendingOf(stopped) == std::vector<int>{3};
    ok = ok && !stopped.state.agentModel.isRestored(Action::East);
    check(counter, ok,
          "C1-6c-1: B 2, S_max 2, V1 work, 3 replays: C1_6C_2, [3] left, not "
          "restored");
}

void checkC1SixC2(TestCounter& counter) {
    const WorldState demo = demoWorld();
    AgentSettings five{2, 5, 100, 3};
    SingleAgent finished(makeObservation(1, demo), five);
    seedModelV1(finished, 3);
    const RunResult run = runSingleAgent(demo, finished, five);
    const std::vector<std::string> expected = {
        "Step 1: safe stop (investigation unfinished, no cautious action)",
        "Step 2: safe stop (investigation unfinished, no cautious action)",
        "  - correction 1: action model for East restored to the standard "
        "rule: CONSISTENT",
        "Step 3: safe stop (budget too small to plan)",
        "Step 4: safe stop (budget too small to plan)",
        "Step 5: safe stop (budget too small to plan)",
        "Result: Stopped -- unresolved after 0 world steps, 5 safe stops",
        "Mismatches: 0",
        "Corrections: 1 (0 verified)",
        "Model evaluations used: 3"};
    bool ok = linesEqual(expected, run.lines, *counter.out);
    ok = ok && pendingOf(finished).empty();
    ok = ok && finished.state.agentModel.isRestored(Action::East);
    check(counter, ok,
          "C1-6c-2: same with S_max 5: exactly C1_6C_5; East restored");
}

void checkC1Committed(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    StepWatch watch;
    SingleAgent first(makeObservation(1, demo), AgentSettings{});
    runSingleAgent(demo, first, AgentSettings{}, watch.watchers());
    SingleAgent second(makeObservation(1, d2), AgentSettings{}, demo);
    runSingleAgent(d2, second, AgentSettings{}, watch.watchers());
    check(counter, isValidWorld(d2) && watch.committed && watch.maxUsed <= 200,
          "C1-7: demos 1-2: every action committed, no step over B");
}

}  // namespace

void runSingleAgentCasesTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Single agent (cases) ---\n";
    }
    checkC1BeliefsAndHistory(counter);
    checkC1Cautious(counter);
    checkC1SixC1(counter);
    checkC1SixC2(counter);
    checkC1Committed(counter);
}

}  // namespace aih
