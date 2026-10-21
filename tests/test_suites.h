// Purpose: Declares one function per test group, so test_main.cpp
// can run each group without knowing what is inside it.
// Introduced in: Stage 0a (rebuild)
// Architecture: Stage 0a has one group. Later stages add one
// declaration here and one call in test_main.cpp per group.

#ifndef AI_HOLOGRAM_TESTS_TEST_SUITES_H
#define AI_HOLOGRAM_TESTS_TEST_SUITES_H

#include "test_support.h"

namespace aih {

// Runs the project info tests (P1, P2) and the test runner
// tests (TS1 to TS3) into the given counter.
void runProjectInfoTests(TestCounter& counter);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_TEST_SUITES_H
