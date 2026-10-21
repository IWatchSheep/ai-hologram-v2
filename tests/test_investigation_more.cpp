// Purpose: More investigation tests (IV9, IV10, IV11), the
// history checks (VF1, VF2, VF3a, VF3b, HC1), the active
// test targets (AT1) and the record defaults (RC1).
// Introduced in: Stage 3d (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld; IV11's
// hand-altered history also holds valid states. Expected
// values come from checks.py (check_investigation).

#include <utility>
#include <vector>

#include "agent/history_checks.h"
#include "agent/investigation.h"
#include "agent/observation.h"
#include "agent/records.h"
#include "agent/step_predictor.h"
#include "agent/verification.h"
#include "agent_test_fixtures.h"
#include "agent_test_support.h"
#include "model/world_model.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

void checkIv9Iv10(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState moved = withAgentAt(demo, Position{3, 4});
    StepPredictor predictor(WorldModel(), 200);
    predictor.startStep();
    const InvestigationResult anomaly = investigateMismatch(
        demo, Action::North, twoObservations(moved, Action::North), predictor,
        applyAction, 1);
    check(counter, isValidWorld(moved) && anomaly.anomaly,
          "IV9: believed agent differs from before: the anomaly flag");
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    StepPredictor tight(WorldModel(), 1);
    tight.startStep();
    const InvestigationResult deferred = investigateMismatch(
        demo, Action::East, twoObservations(d2, Action::East), tight,
        applyAction, 1);
    bool ok = deferred.outcome == InvestigationOutcome::Deferred;
    ok = ok && tight.usedThisStep() == 0;
    check(counter, isValidWorld(d2) && ok,
          "IV10: B 1, reserve 1: Deferred, 0 evaluations");
}

void checkIv11(TestCounter& counter) {
    const WorldState demo = demoWorld();
    WorldState recorded = withAgentAt(demo, Position{4, 3});
    recorded = withObjectAt(recorded, Position{5, 3});
    Observation second = makeObservation(2, recorded);
    second.lastAction = Action::East;
    second.lastOutcome = MoveOutcome::Blocked;
    const std::vector<Observation> history = {makeObservation(1, recorded),
                                              second};
    WorldState belief = withAgentAt(demo, Position{4, 3});
    belief = withObjectAt(belief, Position{5, 4});
    StepPredictor predictor(WorldModel(), 200);
    predictor.startStep();
    const InvestigationResult result = investigateMismatch(
        belief, Action::East, history, predictor, applyAction, 1);
    bool ok = result.outcome == InvestigationOutcome::NoCandidatePassed;
    ok = ok && result.suspected == SourceLabel::S5;
    ok = ok && predictor.usedThisStep() == 1;
    const bool valid = isValidWorld(belief) && isValidWorld(recorded);
    check(counter, valid && ok,
          "IV11: object (5, 4) against a Blocked East record: "
          "NoCandidatePassed, S5, 1 evaluation");
}

void checkVf1Vf2(TestCounter& counter) {
    const std::vector<Observation> history = eastHistory();
    const Observation& o2 = history[2];
    bool ok = objectHistoryCheck(Position{6, 3}, o2);
    ok = ok && !objectHistoryCheck(Position{5, 3}, o2);
    check(counter, isValidWorld(o2.state) && ok,
          "VF1: objectHistoryCheck against o2: (6, 3) passes, (5, 3) fails");
    const WorldState belief = withObjectAt(o2.state, Position{6, 4});
    StepPredictor predictor(WorldModel(), 200);
    predictor.startStep();
    const InvestigationResult result = investigateMismatch(
        belief, Action::East, history, predictor, applyAction, 1);
    bool ok2 =
        result.outcome == InvestigationOutcome::BeliefCandidatesConsistent;
    ok2 = ok2 && result.candidates.size() == 1;
    if (result.candidates.size() == 1) {
        ok2 = ok2 && result.candidates[0].element.kind == ElementKind::Object;
        ok2 = ok2 && result.candidates[0].position == Position{6, 3};
    }
    check(counter, isValidWorld(belief) && ok2,
          "VF2: object wrongly at (6, 4): [object (6, 3)] consistent, "
          "although earlier entries had it elsewhere (time-aware V1)");
}

void checkVf3Hc1(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    const std::vector<Observation> hist = twoObservations(d2, Action::East);
    check(counter,
          isValidWorld(d2) && wallHistoryCheck(Position{4, 3}, true, hist),
          "VF3a: wallHistoryCheck((4, 3), blocked) true when every entry "
          "has the wall");
    const std::vector<Observation> lacking = {makeObservation(1, demo),
                                              hist[1]};
    check(counter, !wallHistoryCheck(Position{4, 3}, true, lacking),
          "VF3b: wallHistoryCheck false when the first entry lacks it");
    const WorldState lt = ltWorld();
    const WorldState later =
        withObjectAt(withAgentAt(lt, Position{6, 7}), Position{8, 7});
    bool ok = trapHistoryPasses(Position{1, 1}, Position{8, 7},
                                {makeObservation(1, lt)});
    ok = ok && !trapHistoryPasses(
                   Position{1, 1}, Position{8, 7},
                   {makeObservation(1, lt), makeObservation(2, later)});
    ok = ok &&
         trapHistoryPasses(Position{1, 1}, Position{8, 7},
                           {makeObservation(1, later), makeObservation(2, lt)});
    check(counter, isValidWorld(lt) && isValidWorld(later) && ok,
          "HC1: trapHistoryPasses: true for LT alone; false when a later "
          "entry has the object on (8, 7); true when that entry is first");
}

void checkAt1Rc1(TestCounter& counter) {
    const WorldState belief = withWallAt(demoWorld(), Position{4, 3}, true);
    const std::vector<std::pair<Position, Action>> targets =
        activeTestTargets(belief, Position{4, 3});
    const std::vector<std::pair<Position, Action>> expected = {
        {Position{4, 4}, Action::North},
        {Position{3, 3}, Action::East},
        {Position{4, 2}, Action::South}};
    check(counter, isValidWorld(belief) && targets == expected,
          "AT1: activeTestTargets(D2-belief, (4, 3)) is exactly (4, 4) "
          "North, (3, 3) East, (4, 2) South");
    const CorrectionRecord record;
    bool ok = record.id == 0 && record.element.kind == ElementKind::Wall;
    ok = ok && record.status == CorrectionStatus::Consistent;
    ok = ok && !record.tested && record.stepConsistent == 0;
    const InvestigationResult result;
    ok = ok && result.outcome == InvestigationOutcome::ModelSuspected;
    ok = ok && result.suspected == SourceLabel::S5;
    check(counter, ok,
          "RC1: new correction defaults (id 0, wall, Consistent, "
          "untested, step 0); InvestigationResult defaults "
          "(ModelSuspected, S5)");
}

}  // namespace

void runInvestigationMoreTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Investigation (more) ---\n";
    }
    checkIv9Iv10(counter);
    checkIv11(counter);
    checkVf1Vf2(counter);
    checkVf3Hc1(counter);
    checkAt1Rc1(counter);
}

}  // namespace aih
