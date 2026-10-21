// Purpose: Implements the C1 run with faults (plan 3.6):
// the Stage 3 run loop itself, with the true history kept
// through onObservation, the injector on the activation
// hook, and the scorer reading the agent at each phase.
// Introduced in: Stage 4c (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. It never copies the run loop: it hands
// runSingleAgent watch-only watchers, so a run with an
// empty event list gives exactly the Stage 3 result, and
// one loop cannot drift from the other. The activation
// hook is scorer.endPhaseA, then the injector, then
// scorer.afterActivation, in that order: a claim of phase
// A is judged before this step's fault exists. The scorer
// only reads the agent; it never writes to it.

#include "experiment/fault_run.h"

#include <functional>
#include <vector>

#include "agent/agent_types.h"
#include "agent/computation_observer.h"
#include "agent/observation.h"
#include "agent/run_loop.h"
#include "agent/single_agent.h"
#include "experiment/fault_apply.h"
#include "experiment/fault_event.h"
#include "experiment/fault_injector.h"
#include "experiment/scorer.h"
#include "world/world_state.h"

namespace aih {
namespace {

// Connects the activation hook: phase A ends, the fault is
// tried, then the scorer records what was applied.
void connectHook(SingleAgent& agent, Scorer& scorer, const ScoredView& view,
                 FaultInjector& injector) {
    agent.state.activationHook = [&scorer, &view, &injector, &agent](int step) {
        scorer.endPhaseA(view, step);
        injector.atActivationPoint(partsOf(agent.state), step);
        scorer.afterActivation(view, injector.records(), step);
    };
}

// Attaches the watch-only observer of the run (IE-1 H3,
// H4): a ComputationObserver whose three functions only
// call the scorer. The agent reads nothing back from it,
// so it cannot pull (a camera, never a chain), and primary
// scoring never depends on it.
void attachScorerObserver(Scorer& scorer, ComputationObserver& observer,
                          const ScoredView& view, SingleAgent& agent) {
    scorer.attachView(view);
    observer.onEvaluation = [&scorer](const WorldState& state, Action action,
                                      const ActionModel* variantModel,
                                      const Prediction& result) {
        scorer.onEvaluation(state, action, variantModel, result);
    };
    observer.onDeadRead = [&scorer](Position cell, bool value) {
        scorer.onDeadRead(cell, value);
    };
    observer.onHeuristic = [&scorer](const WorldState& state, int value) {
        scorer.onHeuristic(state, value);
    };
    agent.state.predictor.setObserver(&observer);
    agent.state.deadCache.observer = &observer;
}

// The watchers the fault run hands to the Stage 3 loop:
// the true history, the scorer first, then the extra
// watchers, which receive the scorer as a const reference.
RunWatchers faultRunWatchers(std::vector<Observation>& trueHistory,
                             Scorer& scorer, const ScoredView& view,
                             FaultInjector& injector,
                             FaultRunWatchers& watchers) {
    RunWatchers runWatchers;
    runWatchers.onObservation = [&trueHistory](const SingleAgent&,
                                               const Observation& observation) {
        trueHistory.push_back(observation);
    };
    runWatchers.afterDecision = [&watchers, &scorer, &view, &injector](
                                    const SingleAgent& watched,
                                    const Observation& observation,
                                    const Decision& decision) {
        scorer.afterDecide(view, decision);
        if (watchers.afterDecision) {
            watchers.afterDecision(watched, observation, decision, injector,
                                   scorer);
        }
    };
    runWatchers.afterFinalCheck = [&watchers, &scorer, &view, &injector](
                                      const SingleAgent& watched,
                                      const Observation& observation) {
        scorer.endFinalCheck(view);
        if (watchers.afterFinalCheck) {
            watchers.afterFinalCheck(watched, observation, injector, scorer);
        }
    };
    return runWatchers;
}

}  // namespace

FaultRunResult runWithFaults(const WorldState& trueInitialState,
                             SingleAgent& agent, AgentSettings settings,
                             const std::vector<FaultEvent>& events,
                             bool observe, FaultRunWatchers watchers) {
    std::vector<Observation> trueHistory;
    FaultInjector injector(events, &trueHistory);
    Scorer scorer(events, trueInitialState, &trueHistory);
    const ScoredView view = viewOf(agent.state);
    ComputationObserver observer;
    if (observe) {
        attachScorerObserver(scorer, observer, view, agent);
    }
    connectHook(agent, scorer, view, injector);
    RunWatchers runWatchers =
        faultRunWatchers(trueHistory, scorer, view, injector, watchers);
    FaultRunResult result;
    result.run = runSingleAgent(trueInitialState, agent, settings, runWatchers);
    // The hook is emptied, so the agent never points at an
    // injector that is about to be destroyed, and the
    // observer is detached from the predictor, the cache
    // and the cached map, so none keeps its address either.
    agent.state.activationHook = std::function<void(int)>{};
    agent.state.predictor.setObserver(nullptr);
    agent.state.deadCache.observer = nullptr;
    agent.state.deadCache.map.observer = nullptr;
    scorer.finish(view);
    result.decisionSteps = agent.state.decisionStep;
    result.activations = injector.records();
    result.eventScores = scorer.eventScores();
    result.score = scorer.runScore();
    result.observedEvaluations = scorer.observedEvaluations();
    return result;
}

}  // namespace aih
