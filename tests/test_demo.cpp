// Purpose: Test for the scripted demonstration (D1, plan
// Section 6.8): its output must match the demo part of plan
// Section 3.8 exactly, from the first empty line to the last.
// Introduced in: Stage 1c (rebuild)
// Architecture: runScriptedDemo takes a stream as a
// parameter, so this test passes a std::ostringstream, a
// stream that collects text in memory, and compares what it
// collected with the exact expected text (checks.py DEMO_TEXT).

#include <sstream>
#include <string>

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

}  // namespace

void runDemoTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Demo ---\n";
    }
    std::ostringstream stream;
    runScriptedDemo(stream);
    check(counter, stream.str() == expectedDemoText(),
          "D1: scripted demo output matches plan 3.8 exactly");
}

}  // namespace aih
