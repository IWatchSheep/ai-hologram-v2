// Purpose: Tests for model verification V1 (VF4a, VF4b,
// VF4c, VF5, plan Section 6.4).
// Introduced in: Stage 3d (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld; VF4c's
// hand-altered history also holds a valid state. Expected
// values come from checks.py (check_investigation).

#include <vector>

#include "agent/model_v1.h"
#include "agent/observation.h"
#include "agent/step_predictor.h"
#include "agent_test_support.h"
#include "model/world_model.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

// One observation with the fields the fixture needs.
Observation makeObservation(int step, const WorldState& state) {
    Observation observation;
    observation.decisionStep = step;
    observation.state = state;
    return observation;
}

// The 4-entry East history of checks.py: the demo world,
// then three East actions, each recorded.
std::vector<Observation> eastHistory() {
    std::vector<Observation> history;
    history.push_back(makeObservation(1, demoWorld()));
    for (int step = 2; step <= 4; step = step + 1) {
        const ActionResult after =
            applyAction(history.back().state, Action::East);
        Observation next = makeObservation(step, after.state);
        next.lastAction = Action::East;
        next.lastOutcome = after.outcome;
        history.push_back(next);
    }
    return history;
}

void checkVf4aVf4b(TestCounter& counter) {
    const std::vector<Observation> history = eastHistory();
    const ModelV1Work work = startModelV1(Action::East, history);
    const std::vector<int> expected = {1, 2, 3};
    bool ok = work.pendingIndices == expected;
    ok = ok && startModelV1(Action::West, history).pendingIndices.empty();
    check(counter, isValidWorld(history[0].state) && ok,
          "VF4a: startModelV1(East) gives 1, 2, 3; West gives none");
    int calls = 0;
    const ActionModel counting = [&calls](const WorldState& state,
                                          Action action) {
        calls = calls + 1;
        return applyAction(state, action);
    };
    StepPredictor predictor(WorldModel(counting), 200);
    predictor.startStep();
    ModelV1Work replay = startModelV1(Action::East, history);
    const V1Progress progress = continueModelV1(replay, history, predictor, 1);
    bool ok2 = progress == V1Progress::Done && predictor.usedThisStep() == 3;
    ok2 = ok2 && calls == 0;
    check(counter, ok2,
          "VF4b: replays are Done with 3 variant evaluations, and the "
          "predictor's own model is never called");
}

void checkVf4c(TestCounter& counter) {
    std::vector<Observation> altered = eastHistory();
    WorldState changed = withObjectAt(altered[2].state, Position{6, 4});
    Observation entry = makeObservation(3, changed);
    entry.lastAction = Action::East;
    entry.lastOutcome = MoveOutcome::Pushed;
    altered[2] = entry;
    StepPredictor predictor(WorldModel(), 200);
    predictor.startStep();
    ModelV1Work work = startModelV1(Action::East, altered);
    const V1Progress progress = continueModelV1(work, altered, predictor, 1);
    check(counter, isValidWorld(changed) && progress == V1Progress::Failed,
          "VF4c: a recorded state altered by hand makes it Failed");
}

void checkVf5(TestCounter& counter) {
    const std::vector<Observation> history = eastHistory();
    ModelV1Work work = startModelV1(Action::East, history);
    StepPredictor predictor(WorldModel(), 3);
    predictor.startStep();
    const V1Progress first = continueModelV1(work, history, predictor, 1);
    bool ok = first == V1Progress::Pending && predictor.usedThisStep() == 2;
    ok = ok && work.pendingIndices == std::vector<int>{3};
    predictor.startStep();
    const V1Progress second = continueModelV1(work, history, predictor, 1);
    ok = ok && second == V1Progress::Done && predictor.usedThisStep() == 1;
    check(counter, isValidWorld(history[0].state) && ok,
          "VF5: B 3, reserve 1: 2 replays, Pending with [3] left; after "
          "startStep(), Done with 1 more");
}

}  // namespace

void runModelV1Tests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Model V1 ---\n";
    }
    checkVf4aVf4b(counter);
    checkVf4c(counter);
    checkVf5(counter);
}

}  // namespace aih
