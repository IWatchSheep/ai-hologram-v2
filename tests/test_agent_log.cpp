// Purpose: Tests for the agent log text (LG1 to LG7, plan
// Section 6.1): every function of plan 3.12 gives its line.
// Introduced in: Stage 3a (rebuild)
// Architecture: Each check builds the lines of one group and
// compares them with the exact expected text of LG_EXPECTED
// in checks.py, using the shared linesEqual helper.

#include <string>
#include <vector>

#include "agent/agent_log.h"
#include "agent_test_support.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"

namespace aih {
namespace {

void checkLg1Lg2(TestCounter& counter) {
    const std::vector<std::string> planLines = {
        planSearchLine(4), planFallbackLine(10), planActiveTestLine(1, 2),
        planCautiousLine()};
    const std::vector<std::string> expectedPlan = {
        "Plan: search found a plan of 4 actions",
        "Plan: fallback toward the best state found (10 actions)",
        "Plan: active test of correction 1 (2 actions)",
        "Plan: cautious mode (investigation unfinished)"};
    check(counter, linesEqual(expectedPlan, planLines, *counter.out),
          "LG1: plan lines match plan 3.12 exactly");
    const std::vector<std::string> stepLines = {
        stepLine(1, Action::East, MoveOutcome::Moved, MoveOutcome::Blocked,
                 false),
        stepLine(12, Action::South, MoveOutcome::Pushed, MoveOutcome::Pushed,
                 true),
        safeStopLine(3, "no safe action")};
    const std::vector<std::string> expectedSteps = {
        "Step 1: East | predicted Moved, observed Blocked | MISMATCH",
        "Step 12: South | predicted Pushed, observed Pushed | match",
        "Step 3: safe stop (no safe action)"};
    check(counter, linesEqual(expectedSteps, stepLines, *counter.out),
          "LG2: step and safe-stop lines match plan 3.12 exactly");
}

void checkLg3Lg4(TestCounter& counter) {
    const std::vector<std::string> v2Lines = {correctionVerifiedLine(1),
                                              correctionRejectedLine(12)};
    const std::vector<std::string> expectedV2 = {"  - correction 1: VERIFIED",
                                                 "  - correction 12: REJECTED"};
    check(counter, linesEqual(expectedV2, v2Lines, *counter.out),
          "LG3: V2 settlement lines match plan 3.12 exactly");
    const std::vector<std::string> investigationLines = {
        investigationWallLine(Position{4, 3}, false, true),
        investigationWallLine(Position{6, 3}, true, false),
        investigationObjectLine(Position{5, 4}, Position{5, 3}),
        investigationModelLine(Action::East),
        anomalyAgentLine(),
        investigationDeferredLine()};
    const std::vector<std::string> expectedInvestigation = {
        "  - investigation: wall at (4, 3) believed free, recorded "
        "blocked (source S2)",
        "  - investigation: wall at (6, 3) believed blocked, recorded "
        "free (source S2)",
        "  - investigation: object believed at (5, 4), recorded at "
        "(5, 3) (source S2)",
        "  - investigation: belief matched the record; action model "
        "for East suspected (source S1)",
        "  - anomaly: believed agent position differs from the record",
        "  - investigation: budget too small for verification; retried "
        "next step"};
    check(counter,
          linesEqual(expectedInvestigation, investigationLines, *counter.out),
          "LG4: investigation lines match plan 3.12 exactly");
}

void checkLg5Lg6(TestCounter& counter) {
    const std::vector<std::string> correctionLines = {
        correctionWallLine(1, Position{4, 3}, true),
        correctionWallLine(2, Position{6, 3}, false),
        correctionObjectLine(3, Position{5, 3}),
        correctionModelLine(4, Action::West),
        correctionTrapLine(5, Position{1, 1}),
        unexplainedLine()};
    const std::vector<std::string> expectedCorrections = {
        "  - correction 1: wall at (4, 3) set to blocked: CONSISTENT",
        "  - correction 2: wall at (6, 3) set to free: CONSISTENT",
        "  - correction 3: object position set to (5, 3): CONSISTENT",
        "  - correction 4: action model for West restored to the "
        "standard rule: CONSISTENT",
        "  - correction 5: (1, 1) classified as a dead square: "
        "CONSISTENT",
        "  - investigation: no candidate passed V1 (source S5); "
        "unexplained update"};
    check(counter,
          linesEqual(expectedCorrections, correctionLines, *counter.out),
          "LG5: correction lines match plan 3.12 exactly");
    const std::vector<std::string> consequenceLines = {
        consequenceSurpriseLine(Position{2, 2}), fullStateDeadEndLine(),
        consequenceMismatchLine(Position{1, 1}), anomalyInvalidLine()};
    const std::vector<std::string> expectedConsequence = {
        "  - consequence surprise: search exhausted with the object "
        "at (2, 2)",
        "  - full-state dead end (dead-square classification "
        "consistent; no claim)",
        "  - consequence mismatch: (1, 1) violates the dead-square "
        "rule (source S1)",
        "  - anomaly: invalid hypothetical state skipped"};
    check(counter,
          linesEqual(expectedConsequence, consequenceLines, *counter.out),
          "LG6: consequence lines match plan 3.12 exactly");
}

void checkLg7(TestCounter& counter) {
    const std::vector<std::string> resultLines = {
        resultLine("Stopped -- unresolved", 0, 2), mismatchesLine(1),
        correctionsLine(3, 2), evaluationsLine(414)};
    const std::vector<std::string> expectedResults = {
        "Result: Stopped -- unresolved after 0 world steps, 2 safe stops",
        "Mismatches: 1", "Corrections: 3 (2 verified)",
        "Model evaluations used: 414"};
    check(counter, linesEqual(expectedResults, resultLines, *counter.out),
          "LG7: result lines match plan 3.12 exactly");
}

}  // namespace

void runAgentLogTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Agent log ---\n";
    }
    checkLg1Lg2(counter);
    checkLg3Lg4(counter);
    checkLg5Lg6(counter);
    checkLg7(counter);
}

}  // namespace aih
