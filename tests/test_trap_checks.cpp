// Purpose: Tests for the trap checks and the consequence
// check (VF9a to VF9d, VF10, VF11, VF12, VF13, plan 6.4).
// Introduced in: Stage 3d (rebuild)
// Architecture: Every fixture is a valid world from
// agent_test_support, checked with isValidWorld. Expected
// values come from checks.py (check_investigation).

#include <vector>

#include "agent/dead_squares.h"
#include "agent/investigation.h"
#include "agent/observation.h"
#include "agent/step_predictor.h"
#include "agent/verification.h"
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

// One observation holding state at step 1.
Observation firstObservation(const WorldState& state) {
    Observation observation;
    observation.decisionStep = 1;
    observation.state = state;
    return observation;
}

// The trap map for LT: the analysis with (1, 1) overridden
// to "not dead", as in checks.py.
DeadSquareMap trapMapForLt(FreeComputationCounts& counts) {
    return computeDeadSquares(ltWorld(), {Position{1, 1}}, {}, counts);
}

void checkVf9aVf9b(TestCounter& counter) {
    const WorldState lt = ltWorld();
    FreeComputationCounts counts;
    const DeadSquareMap map = trapMapForLt(counts);
    const std::vector<Observation> history = {firstObservation(lt)};
    check(counter,
          isValidWorld(lt) &&
              trapV1Check(lt, map, Position{1, 1}, history, counts),
          "VF9a: LT with not-dead (1, 1): trapV1Check((1, 1)) is true");
    std::vector<Action> calls;
    const ActionModel counting = [&calls](const WorldState& state,
                                          Action action) {
        calls.push_back(action);
        return applyAction(state, action);
    };
    StepPredictor predictor(WorldModel(), 200);
    predictor.startStep();
    int anomalies = 0;
    const TrapV2Result result =
        trapV2Test(lt, map, Position{1, 1}, predictor, counting, 1, anomalies);
    bool ok = result == TrapV2Result::Verified && predictor.usedThisStep() == 2;
    const std::vector<Action> expected = {Action::North, Action::West};
    ok = ok && calls == expected;
    check(counter, ok,
          "VF9b: trapV2Test Verified with exactly 2 evaluations through "
          "the current model (North, then West)");
}

void checkVf9cVf9d(TestCounter& counter) {
    const WorldState lt = ltWorld();
    FreeComputationCounts counts;
    const DeadSquareMap map = trapMapForLt(counts);
    StepPredictor tight(WorldModel(), 2);
    tight.startStep();
    int anomalies = 0;
    const TrapV2Result waiting =
        trapV2Test(lt, map, Position{1, 1}, tight, applyAction, 1, anomalies);
    bool ok = waiting == TrapV2Result::Waiting && tight.usedThisStep() == 0;
    check(counter, isValidWorld(lt) && ok,
          "VF9c: B 2, reserve 1: Waiting, 0 evaluations");
    const WorldState demo = demoWorld();
    FreeComputationCounts demoCounts;
    const DeadSquareMap demoMap = computeDeadSquares(demo, {}, {}, demoCounts);
    StepPredictor predictor(WorldModel(), 200);
    predictor.startStep();
    int demoAnomalies = 0;
    const TrapV2Result rejected =
        trapV2Test(demo, demoMap, Position{5, 3}, predictor, applyAction, 1,
                   demoAnomalies);
    bool ok2 = !trapV1Check(demo, demoMap, Position{5, 3},
                            {firstObservation(demo)}, demoCounts);
    ok2 = ok2 && rejected == TrapV2Result::Rejected;
    check(counter, isValidWorld(demo) && ok2,
          "VF9d: D at (5, 3): trapV1Check is false and trapV2Test is "
          "Rejected");
}

void checkVf10(TestCounter& counter) {
    const WorldState lt = ltWorld();
    FreeComputationCounts counts;
    const DeadSquareMap map = trapMapForLt(counts);
    StepPredictor predictor(WorldModel(), 200);
    predictor.startStep();
    int anomalies = 0;
    trapV2Test(lt, map, Position{1, 1}, predictor, applyAction, 1, anomalies);
    bool ok = anomalies == 0;
    ok = ok && isValidWorld(withAgentAt(lt, Position{1, 2}));
    ok = ok && isValidWorld(withAgentAt(lt, Position{2, 1}));
    check(counter, isValidWorld(lt) && ok,
          "VF10: no anomaly in VF9b, and both hypothetical states are "
          "valid");
}

void checkVf11Vf12(TestCounter& counter) {
    const WorldState l2 = l2World();
    FreeComputationCounts counts;
    const DeadSquareMap map = computeDeadSquares(l2, {}, {}, counts);
    const ConsequenceOutcome outcome =
        checkConsequence(l2, map, {firstObservation(l2)}, counts);
    check(counter,
          isValidWorld(l2) && outcome == ConsequenceOutcome::FullStateDeadEnd,
          "VF11: L2: checkConsequence gives FullStateDeadEnd");
    const WorldState lt = ltWorld();
    FreeComputationCounts trapCounts;
    const DeadSquareMap trapMap = trapMapForLt(trapCounts);
    const ConsequenceOutcome mismatch =
        checkConsequence(lt, trapMap, {firstObservation(lt)}, trapCounts);
    check(counter, isValidWorld(lt) && mismatch == ConsequenceOutcome::Mismatch,
          "VF12: LT with the override: Mismatch (labelled S1 by 3.9)");
}

void checkVf13(TestCounter& counter) {
    const WorldState lt = ltWorld();
    const WorldState later =
        withObjectAt(withAgentAt(lt, Position{6, 7}), Position{8, 7});
    Observation second;
    second.decisionStep = 2;
    second.state = later;
    FreeComputationCounts counts;
    const DeadSquareMap trapMap = trapMapForLt(counts);
    bool ok = checkConsequence(lt, trapMap, {firstObservation(lt), second},
                               counts) == ConsequenceOutcome::NotApplicable;
    FreeComputationCounts plainCounts;
    const DeadSquareMap plainMap = computeDeadSquares(lt, {}, {}, plainCounts);
    ok = ok &&
         checkConsequence(lt, plainMap, {firstObservation(lt)}, plainCounts) ==
             ConsequenceOutcome::NotApplicable;
    const WorldState lg = lgWorld();
    FreeComputationCounts lgCounts;
    const DeadSquareMap lgMap = computeDeadSquares(lg, {}, {}, lgCounts);
    ok = ok && checkConsequence(lg, lgMap, {firstObservation(lg)}, lgCounts) ==
                   ConsequenceOutcome::NotApplicable;
    const bool valid = isValidWorld(lt) && isValidWorld(later);
    check(counter, valid && isValidWorld(lg) && ok,
          "VF13: NotApplicable when a later entry has the object on the "
          "destination, when p is dead, and when the object is on the "
          "destination (LG)");
}

}  // namespace

void runTrapChecksTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Trap checks ---\n";
    }
    checkVf9aVf9b(counter);
    checkVf9cVf9d(counter);
    checkVf10(counter);
    checkVf11Vf12(counter);
    checkVf13(counter);
}

}  // namespace aih
