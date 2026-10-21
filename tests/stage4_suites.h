// Purpose: Declares one function per Stage 4 test group,
// so test_main.cpp can run each group without knowing what
// is inside it.
// Introduced in: Stage 4d (rebuild)
// Architecture: tests/test_suites.h reached the 200-line
// limit (rule R1) at Stage 4c, so every Stage 4 group is
// declared here instead of squeezed into it (plan v1.2).
// Each Stage 4 sub-stage adds its groups here.

#ifndef AI_HOLOGRAM_TESTS_STAGE4_SUITES_H
#define AI_HOLOGRAM_TESTS_STAGE4_SUITES_H

#include "test_support.h"

namespace aih {

// Runs the seeded random tests (RN1 to RN8).
void runSeededRandomTests(TestCounter& counter);

// Runs the layout generator tests (LY1 to LY4).
void runLayoutGeneratorTests(TestCounter& counter);

// Runs the layout set tests (LY5, LY6).
void runLayoutSetsTests(TestCounter& counter);

// Runs the fault event tests (FE1, FE4 demonstration, FE5, FE6).
void runFaultEventsTests(TestCounter& counter);

// Runs the fault event set tests (FE2, FE3, FE4 pilot and
// confirmatory; material only, D-080).
void runFaultEventSetsTests(TestCounter& counter);

// Runs the fault apply tests (FI1, FI2).
void runFaultApplyTests(TestCounter& counter);

// Runs the fault apply more tests (FI3, FI4).
void runFaultApplyMoreTests(TestCounter& counter);

// Runs the fault F5 tests (FI5, FI6).
void runFaultF5Tests(TestCounter& counter);

// Runs the fault activation tests (FI7, FI8).
void runFaultActivationTests(TestCounter& counter);

// Runs the fault run tests (FR1, FR4, IE-T1).
void runFaultRunTests(TestCounter& counter);

// Runs the fault run more tests (FR5, FR6, FR7, RL2, RL3).
void runFaultRunMoreTests(TestCounter& counter);

// Runs the scorer tests (SC1, SC2, SC3).
void runScorerTests(TestCounter& counter);

// Runs the scorer life tests (SC4, SC6).
void runScorerLifeTests(TestCounter& counter);

// Runs the shadow tests (SH1, F1 and F2).
void runShadowTests(TestCounter& counter);

// Runs the shadow more tests (SH1, F3 and F4).
void runShadowMoreTests(TestCounter& counter);

// Runs the observed runs tests (SH2, SH3, SH4, SH5).
void runObservedRunsTests(TestCounter& counter);

// Runs the read sites tests (SH6, heuristic, planner,
// cautious choice).
void runReadSitesTests(TestCounter& counter);

// Runs the read sites more tests (SH6, consequence check,
// local check, trap V2 test).
void runReadSitesMoreTests(TestCounter& counter);

// Runs the CSV output tests (CSV1, CSV3, CSV4).
void runCsvOutputTests(TestCounter& counter);

// Runs the fault demo tests (FD1).
void runFaultDemoTests(TestCounter& counter);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_STAGE4_SUITES_H
