// Purpose: Tests for the investigation of a mismatch
// (IV1 to IV7, plan Section 6.4).
// Introduced in: Stage 3d (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld. Expected
// values come from checks.py (check_investigation), and
// each description starts with its check id.

#include <vector>

#include "agent/investigation.h"
#include "agent/observation.h"
#include "agent/records.h"
#include "agent/step_predictor.h"
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

// The two observations of one action: before, and after
// with the action and its outcome recorded.
std::vector<Observation> twoObservations(const WorldState& trueBefore,
                                         Action action) {
    const ActionResult after = applyAction(trueBefore, action);
    Observation first;
    first.decisionStep = 1;
    first.state = trueBefore;
    Observation second;
    second.decisionStep = 2;
    second.state = after.state;
    second.lastAction = action;
    second.lastOutcome = after.outcome;
    return {first, second};
}

// A model that pushes the object North instead of East.
ActionResult pushNorth(const WorldState& state, Action action) {
    const ActionResult standard = applyAction(state, action);
    if (action != Action::East || standard.outcome != MoveOutcome::Pushed) {
        return standard;
    }
    ActionResult wrong = standard;
    wrong.state.object = moved(state.object, Action::North);
    return wrong;
}

// A model under which East always "moves" one cell.
ActionResult alwaysMovedEast(const WorldState& state, Action action) {
    if (action != Action::East) {
        return applyAction(state, action);
    }
    ActionResult result;
    result.state = state;
    result.state.agent = moved(state.agent, Action::East);
    result.outcome = MoveOutcome::Moved;
    return result;
}

bool isWallCandidate(const BeliefCandidate& c, Position cell, bool blocked) {
    bool ok = c.element.kind == ElementKind::Wall && c.element.cell == cell;
    return ok && c.blocked == blocked;
}

bool isObjectCandidate(const BeliefCandidate& c, Position position) {
    return c.element.kind == ElementKind::Object && c.position == position;
}

// One investigation with a fresh predictor (budget 200,
// reserve 1); used receives the evaluations it charged.
InvestigationResult runInvestigation(const WorldState& belief,
                                     const WorldState& trueBefore,
                                     Action action, const ActionModel& model,
                                     int& used) {
    StepPredictor predictor(WorldModel(), 200);
    predictor.startStep();
    InvestigationResult result =
        investigateMismatch(belief, action, twoObservations(trueBefore, action),
                            predictor, model, 1);
    used = predictor.usedThisStep();
    return result;
}

void checkIv1Iv2(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    int used = 0;
    const InvestigationResult r =
        runInvestigation(demo, d2, Action::East, applyAction, used);
    bool ok = r.outcome == InvestigationOutcome::BeliefCandidatesConsistent;
    ok = ok && r.candidates.size() == 1 && r.suspected == SourceLabel::S2;
    if (r.candidates.size() == 1) {
        ok = ok && isWallCandidate(r.candidates[0], Position{4, 3}, true);
    }
    ok = ok && !r.anomaly && used == 1 && isValidWorld(d2);
    check(counter, isValidWorld(demo) && ok,
          "IV1: belief D, true D2, East: consistent, [wall (4, 3) blocked], "
          "S2, no anomaly, 1 evaluation");
    const WorldState t2 = withAgentAt(demo, Position{4, 3});
    const WorldState belief = withWallAt(t2, Position{6, 3}, true);
    const InvestigationResult r2 =
        runInvestigation(belief, t2, Action::East, applyAction, used);
    bool ok2 = r2.outcome == InvestigationOutcome::BeliefCandidatesConsistent;
    ok2 = ok2 && r2.candidates.size() == 1 && r2.suspected == SourceLabel::S2;
    if (r2.candidates.size() == 1) {
        ok2 = ok2 && isWallCandidate(r2.candidates[0], Position{6, 3}, false);
    }
    check(counter, isValidWorld(belief) && ok2,
          "IV2: agent at (4, 3), wrong wall at (6, 3): [wall (6, 3) free], "
          "S2");
}

void checkIv3Iv4(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState belief3 = withAgentAt(demo, Position{4, 3});
    const WorldState true3 = withObjectAt(belief3, Position{5, 4});
    int used = 0;
    const InvestigationResult r3 =
        runInvestigation(belief3, true3, Action::East, applyAction, used);
    bool ok3 = r3.outcome == InvestigationOutcome::BeliefCandidatesConsistent;
    ok3 = ok3 && r3.candidates.size() == 1 && r3.suspected == SourceLabel::S2;
    if (r3.candidates.size() == 1) {
        ok3 = ok3 && isObjectCandidate(r3.candidates[0], Position{5, 4});
    }
    check(counter, isValidWorld(true3) && ok3,
          "IV3: believed object at c1 but not there: [object (5, 4)] from "
          "before, outside the footprint, S2");
    const WorldState belief4 = withObjectAt(belief3, Position{5, 4});
    const InvestigationResult r4 =
        runInvestigation(belief4, belief3, Action::East, applyAction, used);
    bool ok4 = r4.outcome == InvestigationOutcome::BeliefCandidatesConsistent;
    ok4 = ok4 && r4.candidates.size() == 1 && r4.suspected == SourceLabel::S2;
    if (r4.candidates.size() == 1) {
        ok4 = ok4 && isObjectCandidate(r4.candidates[0], Position{5, 3});
    }
    check(counter, isValidWorld(belief4) && ok4,
          "IV4: object at c1 but believed at (5, 4): [object (5, 3)], S2");
}

void checkIv5Iv6(TestCounter& counter) {
    const WorldState t4 = withAgentAt(demoWorld(), Position{4, 3});
    int used = 0;
    const InvestigationResult r5 =
        runInvestigation(t4, t4, Action::East, pushNorth, used);
    bool ok5 = r5.outcome == InvestigationOutcome::ModelSuspected;
    ok5 = ok5 && r5.candidates.empty() && r5.suspected == SourceLabel::S1;
    check(counter, isValidWorld(t4) && ok5 && used == 0,
          "IV5: belief = truth, push-north model: ModelSuspected, no "
          "candidates, S1, 0 evaluations");
    WorldState belief6 = withWallAt(t4, Position{6, 3}, true);
    belief6 = withObjectAt(belief6, Position{5, 4});
    const InvestigationResult r6 =
        runInvestigation(belief6, t4, Action::East, applyAction, used);
    bool ok6 = r6.outcome == InvestigationOutcome::BeliefCandidatesConsistent;
    ok6 = ok6 && r6.candidates.size() == 2 && r6.suspected == SourceLabel::S2;
    if (r6.candidates.size() == 2) {
        ok6 = ok6 && isWallCandidate(r6.candidates[0], Position{6, 3}, false);
        ok6 = ok6 && isObjectCandidate(r6.candidates[1], Position{5, 3});
    }
    check(counter, isValidWorld(belief6) && ok6 && used == 1,
          "IV6: wrong wall (6, 3) and wrong object: [wall free, object "
          "(5, 3)], verified together, 1 evaluation, S2");
}

void checkIv7(TestCounter& counter) {
    const WorldState demo = demoWorld();
    const WorldState d2 = withWallAt(demo, Position{4, 3}, true);
    int used = 0;
    const InvestigationResult r =
        runInvestigation(demo, d2, Action::East, alwaysMovedEast, used);
    bool ok = r.outcome == InvestigationOutcome::NoCandidatePassed;
    check(counter, isValidWorld(demo) && ok && r.suspected == SourceLabel::S5,
          "IV7: current model under which East always moves: "
          "NoCandidatePassed, S5");
}

}  // namespace

void runInvestigationTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Investigation ---\n";
    }
    checkIv1Iv2(counter);
    checkIv3Iv4(counter);
    checkIv5Iv6(counter);
    checkIv7(counter);
}

}  // namespace aih
