// Purpose: Tests for the run loop and the watch-only
// hooks (RL1, OB5, OB6, plan Section 6.5).
// Introduced in: Stage 3e (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld. The
// watchers and the activation hook only watch (rule R6):
// these checks prove a watched run is identical to an
// unwatched one, and count what the hooks report.

#include <string>
#include <vector>

#include "agent/agent_types.h"
#include "agent/computation_observer.h"
#include "agent/observation.h"
#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
#include "expected_demo_lines.h"
#include "model/prediction.h"
#include "model/world_model.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

struct SeenStep {
    int step = 0;
    int logSize = 0;
    PlanKind planKind = PlanKind::None;
};

void checkRl1(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    SingleAgent watchedAgent(makeObservation(1, d2), AgentSettings{}, demo);
    StepWatch watch;
    const RunResult watched =
        runSingleAgent(d2, watchedAgent, AgentSettings{}, watch.watchers());
    SingleAgent plainAgent(makeObservation(1, d2), AgentSettings{}, demo);
    const RunResult plain = runSingleAgent(d2, plainAgent, AgentSettings{});
    bool ok = watch.delivered.size() == 18 && watch.finals.size() == 1;
    ok = ok && plainAgent.state.decisionStep == 18;
    if (watch.finals.size() == 1) {
        ok = ok && watch.finals[0].decisionStep == 19;
    }
    ok = ok && watched.outcome == plain.outcome;
    ok = ok && watched.worldSteps == plain.worldSteps;
    ok = ok && watchedAgent.logLines() == plainAgent.logLines();
    check(counter, isValidWorld(d2) && ok,
          "RL1: watchers on demo 2: 18 decisions and 1 final check "
          "seen (step 19); outcome, steps and log identical without");
}

void checkOb5(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    SingleAgent agent(makeObservation(1, d2), AgentSettings{}, demo);
    std::vector<SeenStep> seen;
    agent.state.activationHook = [&seen, &agent](int step) {
        SeenStep entry;
        entry.step = step;
        entry.logSize = static_cast<int>(agent.logLines().size());
        entry.planKind = agent.state.planKind;
        seen.push_back(entry);
    };
    runSingleAgent(d2, agent, AgentSettings{});
    bool ok = seen.size() == 18;
    if (ok) {
        for (int index = 0; index < 18; index = index + 1) {
            ok = ok && seen[index].step == index + 1;
        }
        ok = ok && seen[1].logSize == 4;
        ok = ok && seen[1].planKind == PlanKind::Goal;
    }
    AgentSettings settings;
    settings.budgetPerStep = 1;
    SingleAgent deferredAgent(makeObservation(1, d2), settings, demo);
    seedCommitted(deferredAgent, Action::East);
    std::vector<int> deferredSeen;
    deferredAgent.state.activationHook = [&deferredSeen](int step) {
        deferredSeen.push_back(step);
    };
    const ActionResult blocked = applyAction(d2, Action::East);
    Observation second = makeObservation(2, blocked.state);
    second.lastAction = Action::East;
    second.lastOutcome = blocked.outcome;
    deferredAgent.decide(second);
    ok = ok && deferredSeen == std::vector<int>{2};
    check(counter, isValidWorld(d2) && ok,
          "OB5: activation hook on demo 2 called once per step "
          "1..18; at step 2 the check's 4 lines are logged, plan "
          "still Goal; deferred case called once (step 2)");
}

void checkOb6(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    SingleAgent agent(makeObservation(1, d2), AgentSettings{}, demo);
    int evaluations = 0;
    ComputationObserver observer;
    observer.onEvaluation = [&evaluations](const WorldState&, Action,
                                           const ActionModel*,
                                           const Prediction&) {
        // Count every reported evaluation (IE-1 H3).
        evaluations = evaluations + 1;
    };
    agent.state.predictor.setObserver(&observer);
    agent.state.deadCache.observer = &observer;
    const RunResult run = runSingleAgent(d2, agent, AgentSettings{});
    bool ok = linesEqual(c1Demo2Lines(), run.lines, *counter.out);
    ok = ok && evaluations == 414;
    check(counter, isValidWorld(d2) && ok,
          "OB6: observers attached to demo 2: run lines still "
          "exactly C1_2, and 414 evaluations are reported");
}

}  // namespace

void runRunLoopTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Run loop ---\n";
    }
    checkRl1(counter);
    checkOb5(counter);
    checkOb6(counter);
}

}  // namespace aih
