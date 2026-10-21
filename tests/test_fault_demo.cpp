// Purpose: Tests for the fault demonstration (FD1; plan
// Section 6.5): runFaultDemo writes exactly the 68 lines.
// Introduced in: Stage 4f (rebuild)
// Architecture: The text runFaultDemo writes to a
// std::ostringstream is split into lines and compared,
// line by line, with the one shared copy of the expected
// lines (expected_fault_demo, rule R4).

#include <sstream>
#include <string>
#include <vector>

#include "agent_test_support.h"
#include "demo/fault_demo.h"
#include "expected_fault_demo.h"
#include "stage4_suites.h"
#include "test_support.h"

namespace aih {
namespace {

// Splits text into its lines, without the newline characters.
std::vector<std::string> splitLines(const std::string& text) {
    std::vector<std::string> lines;
    std::string current;
    for (const char ch : text) {
        if (ch == '\n') {
            lines.push_back(current);
            current.clear();
        } else {
            current.push_back(ch);
        }
    }
    if (!current.empty()) {
        lines.push_back(current);
    }
    return lines;
}

}  // namespace

void runFaultDemoTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Fault demo ---\n";
    }
    std::ostringstream stream;
    runFaultDemo(stream);
    const std::vector<std::string> actual = splitLines(stream.str());
    const bool ok = linesEqual(expectedFaultDemoLines(), actual, *counter.out);
    check(counter, ok, "FD1: runFaultDemo writes exactly the 68 lines");
}

}  // namespace aih
