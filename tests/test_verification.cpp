// Purpose: Tests for the alternative prediction and for
// which model each verification uses (VF7a, VF7b, VF7c,
// VF8a, VF8b, VF8c, plan Section 6.4).
// Introduced in: Stage 3d (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld. Expected
// values come from checks.py (check_investigation).

#include <optional>
#include <vector>

#include "agent/investigation.h"
#include "agent/observation.h"
#include "agent/records.h"
#include "agent/step_predictor.h"
#include "agent/verification.h"
#include "agent_test_support.h"
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

// The wall correction of checks.py wall_record().
CorrectionRecord wallRecord() {
    CorrectionRecord record;
    record.id = 1;
    record.element.kind = ElementKind::Wall;
    record.element.cell = Position{4, 3};
    record.newBlocked = true;
    record.oldBlocked = false;
    record.stepConsistent = 1;
    return record;
}

// The model correction of checks.py model_record().
CorrectionRecord modelRecord() {
    CorrectionRecord record;
    record.id = 2;
    record.element.kind = ElementKind::ActionModel;
    record.element.action = Action::East;
    record.stepConsistent = 1;
    return record;
}

// The object correction of checks.py object_record().
CorrectionRecord objectRecord() {
    CorrectionRecord record;
    record.id = 3;
    record.element.kind = ElementKind::Object;
    record.newPosition = Position{5, 3};
    record.oldPosition = Position{5, 4};
    record.stepConsistent = 1;
    return record;
}

// A fresh predictor with budget 200, already started.
StepPredictor startedPredictor() {
    StepPredictor predictor(WorldModel(), 200);
    predictor.startStep();
    return predictor;
}

void checkVf7aVf7b(TestCounter& counter) {
    const WorldState demo = demoWorld();
    StepPredictor east = startedPredictor();
    const std::optional<Prediction> wallEast = alternativePrediction(
        wallRecord(), demo, Action::East, east, applyAction, applyAction);
    StepPredictor north = startedPredictor();
    const std::optional<Prediction> wallNorth = alternativePrediction(
        wallRecord(), demo, Action::North, north, applyAction, applyAction);
    bool ok = wallEast.has_value() && east.usedThisStep() == 1;
    ok = ok && !wallNorth.has_value() && north.usedThisStep() == 0;
    check(counter, isValidWorld(demo) && ok,
          "VF7a: wall correction at (4, 3): alternative for East (1 "
          "evaluation), none for North (0)");
    StepPredictor modelEastP = startedPredictor();
    const std::optional<Prediction> modelEast =
        alternativePrediction(modelRecord(), demo, Action::East, modelEastP,
                              applyAction, applyAction);
    StepPredictor modelWestP = startedPredictor();
    const std::optional<Prediction> modelWest =
        alternativePrediction(modelRecord(), demo, Action::West, modelWestP,
                              applyAction, applyAction);
    bool ok2 = modelEast.has_value() && modelEastP.usedThisStep() == 1;
    ok2 = ok2 && !modelWest.has_value() && modelWestP.usedThisStep() == 0;
    check(counter, isValidWorld(demo) && ok2,
          "VF7b: model(East) correction: one for East, none for West");
}

void checkVf7c(TestCounter& counter) {
    WorldState belief = withAgentAt(demoWorld(), Position{4, 4});
    belief = withObjectAt(belief, Position{5, 3});
    StepPredictor nearP = startedPredictor();
    const std::optional<Prediction> nearAlt = alternativePrediction(
        objectRecord(), belief, Action::East, nearP, applyAction, applyAction);
    StepPredictor farP = startedPredictor();
    const std::optional<Prediction> farAlt = alternativePrediction(
        objectRecord(), belief, Action::West, farP, applyAction, applyAction);
    bool ok = nearAlt.has_value() && nearAlt->outcome == MoveOutcome::Pushed;
    if (nearAlt.has_value()) {
        ok = ok && nearAlt->state.agent == Position{5, 4};
        ok = ok && nearAlt->state.object == Position{6, 4};
    }
    ok = ok && nearP.usedThisStep() == 1;
    ok = ok && !farAlt.has_value() && farP.usedThisStep() == 0;
    check(counter, isValidWorld(belief) && ok,
          "VF7c: object correction (old (5, 4)), agent at (4, 4): "
          "alternative for East, Pushed, agent (5, 4), object (6, 4), "
          "1 evaluation; none for West");
}

void checkVf8a(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    const ActionResult after = applyAction(d2, Action::East);
    Observation first;
    first.decisionStep = 1;
    first.state = d2;
    Observation second;
    second.decisionStep = 2;
    second.state = after.state;
    second.lastAction = Action::East;
    second.lastOutcome = after.outcome;
    int calls = 0;
    const ActionModel counting = [&calls](const WorldState& state,
                                          Action action) {
        calls = calls + 1;
        return applyAction(state, action);
    };
    StepPredictor predictor(WorldModel(), 200);
    predictor.startStep();
    const InvestigationResult result = investigateMismatch(
        demo, Action::East, {first, second}, predictor, counting, 1);
    bool ok =
        result.outcome == InvestigationOutcome::BeliefCandidatesConsistent;
    ok = ok && calls == 1 && predictor.usedThisStep() == 1;
    check(counter, isValidWorld(d2) && ok,
          "VF8a: the investigation's re-prediction calls the current "
          "model exactly once");
}

void checkVf8b(TestCounter& counter) {
    const WorldState demo = demoWorld();
    int oldCalls = 0;
    int currentCalls = 0;
    const ActionModel oldCounting = [&oldCalls](const WorldState& state,
                                                Action action) {
        oldCalls = oldCalls + 1;
        return applyAction(state, action);
    };
    const ActionModel currentCounting = [&currentCalls](const WorldState& state,
                                                        Action action) {
        currentCalls = currentCalls + 1;
        return applyAction(state, action);
    };
    StepPredictor altP = startedPredictor();
    const std::optional<Prediction> alt = alternativePrediction(
        modelRecord(), demo, Action::East, altP, currentCounting, oldCounting);
    bool ok2 = alt.has_value() && oldCalls == 1 && currentCalls == 0;
    ok2 = ok2 && altP.usedThisStep() == 1;
    check(counter, ok2,
          "VF8b: a model correction's alternative calls the old model "
          "once and never the current one");
}

void checkVf8c(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const Prediction base{Action::East, MoveOutcome::Moved, demo};
    const Prediction otherOutcome{Action::East, MoveOutcome::Blocked, demo};
    const WorldState moved = withAgentAt(demo, Position{4, 3});
    const Prediction otherAgent{Action::East, MoveOutcome::Moved, moved};
    bool ok = !predictionsDiffer(base, base);
    ok = ok && predictionsDiffer(base, otherOutcome);
    ok = ok && predictionsDiffer(base, otherAgent);
    check(counter, isValidWorld(demo) && isValidWorld(moved) && ok,
          "VF8c: predictionsDiffer: identical, no; different outcome, "
          "yes; different agent, yes");
}

}  // namespace

void runVerificationTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Verification ---\n";
    }
    checkVf7aVf7b(counter);
    checkVf7c(counter);
    checkVf8a(counter);
    checkVf8b(counter);
    checkVf8c(counter);
}

}  // namespace aih
