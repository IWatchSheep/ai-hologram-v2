// Purpose: Tests for the project info functions (P1, P2) and
// for the test runner itself (TS1 to TS3), as in plan 6.1.
// Introduced in: Stage 0a (rebuild)
// Architecture: The TS tests use their own counter writing to a
// string stream, so they can check exactly what check() prints
// without changing the real counter's counts.

#include <sstream>
#include <string>

#include "core/project_info.h"
#include "test_suites.h"
#include "test_support.h"

namespace aih {

// TS1: a passing check prints nothing when verbose is false.
bool quietPassWritesNothing() {
    std::ostringstream stream;
    TestCounter local;
    local.out = &stream;
    local.verbose = false;
    check(local, true, "x");
    return stream.str().empty();
}

// TS2: a failing check prints exactly "FAIL: x" plus a newline,
// and records one failure on its own counter only.
bool failureWritesFailLine() {
    std::ostringstream stream;
    TestCounter local;
    local.out = &stream;
    local.verbose = false;
    check(local, false, "x");
    if (stream.str() != "FAIL: x\n") {
        return false;
    }
    return local.failures == 1;
}

// TS3: a passing check prints exactly "PASS: y" plus a newline
// when verbose is true.
bool verbosePassWritesPassLine() {
    std::ostringstream stream;
    TestCounter local;
    local.out = &stream;
    local.verbose = true;
    check(local, true, "y");
    return stream.str() == "PASS: y\n";
}

void runProjectInfoTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Project info ---\n";
    }
    check(counter, projectName() == "AI Hologram",
          "P1: projectName returns AI Hologram");
    check(counter, currentStage() == 0, "P2: currentStage returns 0");
    check(counter, quietPassWritesNothing(),
          "TS1: quiet passing check writes nothing");
    check(counter, failureWritesFailLine(),
          "TS2: failing check writes FAIL line and counts 1");
    check(counter, verbosePassWritesPassLine(),
          "TS3: verbose passing check writes PASS line");
}

}  // namespace aih
