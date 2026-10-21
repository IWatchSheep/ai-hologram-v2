// Purpose: Declares one function per test group, so test_main.cpp
// can run each group without knowing what is inside it.
// Introduced in: Stage 0a (rebuild)
// Architecture: Later stages add one declaration here and one
// call in test_main.cpp per group. The two snippet helpers are
// declared here too, so every checker test compares results
// in exactly the same way (one shared helper per job, R4).

#ifndef AI_HOLOGRAM_TESTS_TEST_SUITES_H
#define AI_HOLOGRAM_TESTS_TEST_SUITES_H

#include <string>
#include <utility>
#include <vector>

#include "checker.h"
#include "test_support.h"

namespace aih {

// Runs the project info tests (P1, P2) and the test runner
// tests (TS1 to TS3) into the given counter.
void runProjectInfoTests(TestCounter& counter);

// Runs the code checker size tests (CK1 to CK52, size rules).
void runCodeCheckerSizeTests(TestCounter& counter);

// Runs the code checker style tests (braces, statements, raw
// strings and the formatted output line CK28).
void runCodeCheckerStyleTests(TestCounter& counter);

// Runs the code checker declaration tests (one declaration
// per statement, templates, enums and lambda bodies).
void runCodeCheckerDeclarationTests(TestCounter& counter);

// Runs the architecture tests (agent includes) and the FS
// tests for which files checkedKind checks.
void runCodeCheckerArchitectureTests(TestCounter& counter);

// Runs the world parse tests (T1 to T20).
void runWorldParseTests(TestCounter& counter);

// Runs the world render tests (RT1 to RT3).
void runWorldRenderTests(TestCounter& counter);

// Runs the world state tests (V1 to V5, V6a to V6d, N1).
void runWorldStateTests(TestCounter& counter);

// Runs the world golden tests (G2 in Stage 1a; GN1 to GN7).
void runWorldGoldenTests(TestCounter& counter);

// Runs the rules tests (R1a to R11b and N2).
void runRulesTests(TestCounter& counter);

// Runs the environment tests (E1 to E4).
void runEnvironmentTests(TestCounter& counter);

// Runs the world golden rules tests (G1 and GR1 to GR5).
void runWorldGoldenRulesTests(TestCounter& counter);

// Runs the demo tests (D1).
void runDemoTests(TestCounter& counter);

// Runs the budget counter tests (BC1 to BC4b).
void runBudgetCounterTests(TestCounter& counter);

// Runs the world model tests (WM1a to WM8).
void runWorldModelTests(TestCounter& counter);

// Runs the state comparison tests (CM1 to CM6).
void runCompareStatesTests(TestCounter& counter);

// Runs the prediction comparison tests (CM7 to CM10).
void runComparePredictionsTests(TestCounter& counter);

// Runs the prediction golden tests (G3, PN1 to PN7).
void runPredictionGoldenTests(TestCounter& counter);

// Runs the footprint tests (FP1, FP10 to FP13).
void runFootprintTests(TestCounter& counter);

// Runs the footprint row tests (FP2 to FP9).
void runFootprintRowsTests(TestCounter& counter);

// Runs the agent log tests (LG1 to LG7).
void runAgentLogTests(TestCounter& counter);

// Runs the dead-square tests (DS1, DS2a, DS2b, DS3a, DS4,
// DS5a, DS5b).
void runDeadSquaresTests(TestCounter& counter);

// Runs the cache and heuristic tests (DS6 to DS8d).
void runHeuristicTests(TestCounter& counter);

// Runs the dead-square observer tests (OB1 to OB3).
void runDeadSquareObserversTests(TestCounter& counter);

// Runs the step predictor tests (SP1 to SP6, OB4).
void runStepPredictorTests(TestCounter& counter);

// Runs the agent model tests (AM1 to AM5).
void runAgentModelTests(TestCounter& counter);

// Runs the planner tests (PL1, PL2a, PL2b, PL3, PL8, PL9, PL10).
void runPlannerTests(TestCounter& counter);

// Runs the planner budget tests (PL4a, PL4b, PL5a, PL5b, PL6, PL7).
void runPlannerBudgetTests(TestCounter& counter);

// Runs the investigation tests (IV1 to IV7).
void runInvestigationTests(TestCounter& counter);

// Runs the further investigation tests (IV9 to IV11, VF1,
// VF2, VF3a, VF3b, HC1, AT1, RC1).
void runInvestigationMoreTests(TestCounter& counter);

// Runs the model V1 tests (VF4a, VF4b, VF4c, VF5).
void runModelV1Tests(TestCounter& counter);

// Runs the verification tests (VF7a to VF7c, VF8a to VF8c).
void runVerificationTests(TestCounter& counter);

// Runs the trap and consequence tests (VF9a to VF13).
void runTrapChecksTests(TestCounter& counter);

// One checker snippet check: checkText on text must return
// exactly the expected (line, rule) pairs, in order.
void checkSnippet(TestCounter& counter, const std::string& description,
                  const std::string& path, const std::string& text,
                  FileKind kind,
                  const std::vector<std::pair<int, std::string>>& expected);

// One checkedKind check: the path must give the expected kind.
void checkKind(TestCounter& counter, const std::string& description,
               const std::string& path, FileKind expected);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_TEST_SUITES_H
