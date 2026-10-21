// Purpose: Tests for the demonstrations (D1, plan Section
// 6.8, and D2, plan Section 6.6): their output must match
// plan Section 3.8 exactly.
// Introduced in: Stage 1c (rebuild); D2 in Stage 2c
// Architecture: Both demos take a stream as a parameter, so
// these tests pass a std::ostringstream, a stream that
// collects text in memory, and compare what it collected
// with the exact expected text (checks.py DEMO_TEXT and
// DEMO2_TEXT).

#include <sstream>
#include <string>

#include "demo/prediction_demo.h"
#include "demo/scripted_demo.h"
#include "test_suites.h"
#include "test_support.h"

namespace aih {
namespace {

// The exact text D1 expects, written out line by line. No
// raw strings are used, as the code rules require.
std::string expectedDemoText() {
    return "\nInitial world:\n"
           "##########\n"
           "#........#\n"
           "#........#\n"
           "#..A.O..D#\n"
           "#........#\n"
           "#........#\n"
           "#...##...#\n"
           "#........#\n"
           "#........#\n"
           "##########\n"
           "\n"
           "Step 1: East -> Moved\n"
           "Step 2: East -> Pushed\n"
           "Step 3: East -> Pushed\n"
           "Step 4: East -> Pushed\n"
           "Step 5: East -> Blocked\n"
           "\nFinal world:\n"
           "##########\n"
           "#........#\n"
           "#........#\n"
           "#......A*#\n"
           "#........#\n"
           "#........#\n"
           "#...##...#\n"
           "#........#\n"
           "#........#\n"
           "##########\n"
           "Goal reached: yes\n"
           "Steps taken: 5\n";
}

// The exact text D2 expects, written out line by line. No
// raw strings are used, as the code rules require.
std::string expectedPredictionText() {
    return "\nPrediction demo 1: correct belief\n"
           "Step 1: East | predicted Moved, observed Moved | match\n"
           "Step 2: East | predicted Pushed, observed Pushed | match\n"
           "Step 3: East | predicted Pushed, observed Pushed | match\n"
           "Step 4: East | predicted Pushed, observed Pushed | match\n"
           "Step 5: East | predicted Blocked, observed Blocked | match\n"
           "Mismatches: 0\n"
           "Model evaluations used: 5\n"
           "\nPrediction demo 2: belief wrongly includes a wall at (6, 3)\n"
           "Step 1: East | predicted Moved, observed Moved | MISMATCH\n"
           "  - wall at (6, 3): predicted blocked, observed free\n"
           "Step 2: East | predicted Pushed, observed Pushed | match\n"
           "Step 3: East | predicted Pushed, observed Pushed | match\n"
           "Step 4: East | predicted Pushed, observed Pushed | match\n"
           "Step 5: East | predicted Blocked, observed Blocked | match\n"
           "Mismatches: 1\n"
           "Model evaluations used: 5\n"
           "Note: in these demos the belief is replaced by the observation "
           "after every step.\n"
           "How beliefs are corrected is decided in Stage 3.\n";
}

}  // namespace

void runDemoTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Demo ---\n";
    }
    std::ostringstream stream;
    runScriptedDemo(stream);
    check(counter, stream.str() == expectedDemoText(),
          "D1: scripted demo output matches plan 3.8 exactly");
    std::ostringstream predictionStream;
    runPredictionDemos(predictionStream);
    check(counter, predictionStream.str() == expectedPredictionText(),
          "D2: prediction demo output matches plan 3.8 exactly");
}

}  // namespace aih
