// Purpose: Tests for the step predictor and the IE-1 H3
// observer (SP1, SP2a, SP2b, SP3, SP4a, SP4b, SP5, SP6, OB4,
// plan Section 6.3).
// Introduced in: Stage 3c (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld. Expected
// values come from checks.py (check_predictor).

#include <optional>
#include <string>
#include <vector>

#include "agent/computation_observer.h"
#include "agent/step_predictor.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
#include "core/text_words.h"
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

void checkSp1(TestCounter& counter) {
    const WorldState demo = demoWorld();
    StepPredictor predictor(WorldModel(), 10);
    predictor.startStep();
    const std::optional<Prediction> first =
        predictor.predict(demo, Action::East);
    bool ok = first.has_value() && predictor.usedThisStep() == 1;
    ok = ok && predictor.remaining() == 9;
    ok = ok && predictor.isCached(demo, Action::East);
    const std::optional<Prediction> second =
        predictor.predict(demo, Action::East);
    ok = ok && second.has_value() && second->state == first->state;
    ok = ok && second->outcome == first->outcome;
    ok = ok && predictor.usedThisStep() == 1;
    check(counter, isValidWorld(demo) && ok,
          "SP1: uncached prediction costs 1 (remaining 9), then cached "
          "and free");
}

void checkSp2(TestCounter& counter) {
    const WorldState demo = demoWorld();
    StepPredictor predictor(WorldModel(), 10);
    predictor.startStep();
    predictor.predict(demo, Action::East);
    predictor.startStep();
    bool okA = !predictor.isCached(demo, Action::East);
    okA = okA && predictor.predict(demo, Action::East).has_value();
    okA = okA && predictor.usedThisStep() == 1;
    check(counter, isValidWorld(demo) && okA,
          "SP2a: startStep() clears the cache; same prediction costs again");
    const WorldState walled = withWallAt(demo, Position{2, 7}, true);
    bool okB = !predictor.isCached(walled, Action::East);
    predictor.predict(walled, Action::East);
    okB = okB && predictor.usedThisStep() == 2;
    okB = okB && predictor.isCached(walled, Action::East);
    predictor.clearCache();
    okB = okB && !predictor.isCached(walled, Action::East);
    check(counter, isValidWorld(walled) && okB,
          "SP2b: other walls not served from cache, cost 1 (used 2); "
          "clearCache() empties");
}

void checkSp3(TestCounter& counter) {
    const WorldState demo = demoWorld();
    StepPredictor predictor(WorldModel(), 5);
    predictor.startStep();
    predictor.predict(demo, Action::East);
    predictor.startStep();
    bool ok = predictor.remaining() == 5 && predictor.usedThisStep() == 0;
    ok = ok && predictor.usedTotal() == 1;
    check(counter, isValidWorld(demo) && ok,
          "SP3: after startStep(), remaining is B (5), used 0, usedTotal 1");
}

void checkSp4(TestCounter& counter) {
    const WorldState demo = demoWorld();
    StepPredictor predictor(WorldModel(), 1);
    predictor.startStep();
    const std::optional<Prediction> first =
        predictor.predict(demo, Action::East);
    const std::optional<Prediction> refused =
        predictor.predict(demo, Action::South);
    bool ok = first.has_value() && !refused.has_value();
    ok = ok && predictor.usedThisStep() == 1 && predictor.remaining() == 0;
    ok = ok && !predictor.isCached(demo, Action::South);
    check(counter, isValidWorld(demo) && ok,
          "SP4a: B 1: second prediction refused, nothing charged or cached");
    const std::optional<Prediction> variant =
        predictor.predictVariant(demo, Action::South, applyAction);
    const bool okB = !variant.has_value() && predictor.usedThisStep() == 1;
    check(counter, okB,
          "SP4b: a variant is refused too when the budget is used up");
}

void checkSp5(TestCounter& counter) {
    const WorldState demo = demoWorld();
    StepPredictor predictor(WorldModel(), 10);
    predictor.startStep();
    const std::optional<Prediction> ordinary =
        predictor.predict(demo, Action::East);
    const std::optional<Prediction> variant =
        predictor.predictVariant(demo, Action::East, alwaysBlocked);
    const std::optional<Prediction> again =
        predictor.predictVariant(demo, Action::East, alwaysBlocked);
    const std::optional<Prediction> cached =
        predictor.predict(demo, Action::East);
    const std::optional<Prediction> south =
        predictor.predictVariant(demo, Action::South, alwaysBlocked);
    bool ok = ordinary.has_value() && ordinary->outcome == MoveOutcome::Moved;
    ok = ok && variant.has_value() && variant->outcome == MoveOutcome::Blocked;
    ok = ok && again.has_value() && cached.has_value();
    ok = ok && cached->outcome == MoveOutcome::Moved && south.has_value();
    ok = ok && predictor.usedThisStep() == 4;
    ok = ok && !predictor.isCached(demo, Action::South);
    check(counter, isValidWorld(demo) && ok,
          "SP5: variant ignores and never fills cache, always costs 1; "
          "ordinary entry stays");
}

void checkSp6(TestCounter& counter) {
    const WorldState demo = demoWorld();
    int calls = 0;
    const ActionModel counting = [&calls](const WorldState& state,
                                          Action action) {
        calls = calls + 1;
        return applyAction(state, action);
    };
    StepPredictor predictor(WorldModel(counting), 10);
    predictor.startStep();
    predictor.predict(demo, Action::East);
    predictor.predict(demo, Action::East);
    predictor.predictVariant(demo, Action::East, counting);
    const bool ok = calls == 2 && predictor.usedThisStep() == 2;
    check(counter, isValidWorld(demo) && ok && predictor.usedTotal() == 2,
          "SP6: counting model: two calls for miss, hit and variant; "
          "used 2, usedTotal 2");
}

void checkOb4(TestCounter& counter) {
    const WorldState demo = demoWorld();
    std::vector<std::string> events;
    ComputationObserver observer;
    observer.onEvaluation = [&events](const WorldState& state, Action action,
                                      const ActionModel* model,
                                      const Prediction& prediction) {
        const std::string kind = model != nullptr ? " variant " : " own ";
        const std::string text = "evaluation " + positionText(state.agent) +
                                 " " + positionText(state.object) + " " +
                                 actionName(action) + kind +
                                 outcomeName(prediction.outcome);
        events.push_back(text);
    };
    StepPredictor predictor(WorldModel(), 3);
    predictor.setObserver(&observer);
    predictor.startStep();
    predictor.predict(demo, Action::East);
    predictor.predict(demo, Action::East);
    predictor.predictVariant(demo, Action::North, alwaysBlocked);
    predictor.predict(demo, Action::South);
    predictor.predict(demo, Action::West);
    const std::vector<std::string> expected = {
        "evaluation (3, 3) (5, 3) East own Moved",
        "evaluation (3, 3) (5, 3) North variant Blocked",
        "evaluation (3, 3) (5, 3) South own Moved"};
    check(
        counter,
        isValidWorld(demo) && events == expected && predictor.usedTotal() == 3,
        "OB4: observer reports exactly East own, North variant, South "
        "own; usedTotal 3");
}

}  // namespace

void runStepPredictorTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Step predictor ---\n";
    }
    checkSp1(counter);
    checkSp2(counter);
    checkSp3(counter);
    checkSp4(counter);
    checkSp5(counter);
    checkSp6(counter);
    checkOb4(counter);
}

}  // namespace aih
