// Purpose: Tests for the agent demonstrations (AD1, plan
// Section 6.6).
// Introduced in: Stage 3f (rebuild)
// Architecture: The text runAgentDemos writes to a
// std::ostringstream is compared, line by line, with the
// expected 46 lines built from the one shared copy of the
// two demos' run lines (expected_demo_lines, rule R4).

#include <sstream>
#include <string>
#include <vector>

#include "agent_test_support.h"
#include "demo/agent_demo.h"
#include "expected_demo_lines.h"
#include "test_suites.h"
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

// The expected 46 lines of AD1_TEXT (checks.py), in order.
std::vector<std::string> expectedDemoTextLines() {
    std::vector<std::string> lines;
    lines.push_back("");
    lines.push_back(
        "Single-agent demonstrations use illustrative settings: "
        "budget 200 per step,");
    lines.push_back(
        "safe-stop limit 5, step limit 100, active-test delay 3 "
        "(not the experiment's values).");
    lines.push_back("");
    lines.push_back("C1 demo 1: correct belief");
    const std::vector<std::string> demo1 = c1Demo1Lines();
    lines.insert(lines.end(), demo1.begin(), demo1.end());
    lines.push_back("");
    lines.push_back("C1 demo 2: belief wrongly has no wall at (4, 3)");
    const std::vector<std::string> demo2 = c1Demo2Lines();
    lines.insert(lines.end(), demo2.begin(), demo2.end());
    lines.push_back(
        "Note: demo 2's wrong belief is hand-written; it is not "
        "fault injection.");
    return lines;
}

}  // namespace

void runAgentDemoTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Agent demo ---\n";
    }
    std::ostringstream stream;
    runAgentDemos(stream);
    const std::vector<std::string> actual = splitLines(stream.str());
    const bool ok = linesEqual(expectedDemoTextLines(), actual, *counter.out);
    check(counter, ok,
          "AD1: runAgentDemos writes exactly AD1_TEXT "
          "(46 lines, 19 and 414 evaluations)");
}

}  // namespace aih
