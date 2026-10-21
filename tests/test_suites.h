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

// Runs the world golden tests (G2 in Stage 1a; G1 in 1b).
void runWorldGoldenTests(TestCounter& counter);

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
