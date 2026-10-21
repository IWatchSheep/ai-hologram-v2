// Purpose: The test program entry point. It runs every test
// group, prints the summary line and reports failures by its
// exit code.
// Introduced in: Stage 0a (rebuild)
// Architecture: Groups are declared in test_suites.h. Output is
// quiet unless the first argument is --verbose (rule R15).
// The snippet helpers live here because every checker group
// uses them and they belong to no single group.

#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "checker.h"
#include "stage4_suites.h"
#include "test_suites.h"
#include "test_support.h"

namespace aih {

void checkSnippet(TestCounter& counter, const std::string& description,
                  const std::string& path, const std::string& text,
                  FileKind kind,
                  const std::vector<std::pair<int, std::string>>& expected) {
    std::vector<Violation> violations = checkText(path, text, kind);
    std::vector<std::pair<int, std::string>> actual;
    for (const Violation& violation : violations) {
        actual.push_back(std::make_pair(violation.line, violation.rule));
    }
    check(counter, actual == expected, description);
}

void checkKind(TestCounter& counter, const std::string& description,
               const std::string& path, FileKind expected) {
    check(counter, checkedKind(path) == expected, description);
}

// Runs the agent test groups (Stages 3a to 3c), so main stays
// a short list of groups.
void runAgentTestGroups(TestCounter& counter) {
    runFootprintTests(counter);
    runFootprintRowsTests(counter);
    runAgentLogTests(counter);
    runDeadSquaresTests(counter);
    runHeuristicTests(counter);
    runDeadSquareObserversTests(counter);
    runStepPredictorTests(counter);
    runAgentModelTests(counter);
    runPlannerTests(counter);
    runPlannerBudgetTests(counter);
    runInvestigationTests(counter);
    runInvestigationMoreTests(counter);
    runModelV1Tests(counter);
    runVerificationTests(counter);
    runTrapChecksTests(counter);
    runSingleAgentTests(counter);
    runSingleAgentCasesTests(counter);
    runSingleAgentBudgetTests(counter);
    runRunLoopTests(counter);
    runNaiveAgentTests(counter);
    runAgentDemoTests(counter);
    runTraceReaderTests(counter);
    runTraceGateTests(counter);
}

// Runs the Stage 4 test groups (4a: seeded random and
// layouts; 4b: fault events), so main stays a short list.
void runStage4TestGroups(TestCounter& counter) {
    runSeededRandomTests(counter);
    runLayoutGeneratorTests(counter);
    runLayoutSetsTests(counter);
    runFaultEventsTests(counter);
    runFaultEventSetsTests(counter);
    runFaultApplyTests(counter);
    runFaultApplyMoreTests(counter);
    runFaultF5Tests(counter);
    runFaultActivationTests(counter);
    runFaultRunTests(counter);
    runFaultRunMoreTests(counter);
    runScorerTests(counter);
    runScorerLifeTests(counter);
}

}  // namespace aih

int main(int argc, char* argv[]) {
    aih::TestCounter counter;
    if (argc > 1) {
        if (std::string(argv[1]) == "--verbose") {
            counter.verbose = true;
        }
    }
    aih::runProjectInfoTests(counter);
    aih::runCodeCheckerSizeTests(counter);
    aih::runCodeCheckerStyleTests(counter);
    aih::runCodeCheckerDeclarationTests(counter);
    aih::runCodeCheckerArchitectureTests(counter);
    aih::runWorldParseTests(counter);
    aih::runWorldRenderTests(counter);
    aih::runWorldStateTests(counter);
    aih::runWorldGoldenTests(counter);
    aih::runRulesTests(counter);
    aih::runEnvironmentTests(counter);
    aih::runWorldGoldenRulesTests(counter);
    aih::runDemoTests(counter);
    aih::runBudgetCounterTests(counter);
    aih::runWorldModelTests(counter);
    aih::runCompareStatesTests(counter);
    aih::runComparePredictionsTests(counter);
    aih::runPredictionGoldenTests(counter);
    aih::runAgentTestGroups(counter);
    aih::runStage4TestGroups(counter);
    std::cout << counter.checks << " checks, " << counter.failures
              << " failures\n";
    if (counter.failures == 0) {
        return 0;
    }
    return 1;
}
