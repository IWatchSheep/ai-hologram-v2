// Purpose: Defines check(), the one helper every test uses to
// record a result and print PASS or FAIL lines as needed.
// Introduced in: Stage 0a (rebuild)
// Architecture: Output is quiet by default (rule R15): only
// failures and the final summary are printed unless verbose.

#include "test_support.h"

#include <string>

namespace aih {

void check(TestCounter& counter, bool condition,
           const std::string& description) {
    counter.checks = counter.checks + 1;
    if (condition) {
        if (counter.verbose) {
            *counter.out << "PASS: " << description << "\n";
        }
    } else {
        counter.failures = counter.failures + 1;
        *counter.out << "FAIL: " << description << "\n";
    }
}

std::string repeatLine(const std::string& text, int count) {
    std::string result;
    for (int i = 0; i < count; i = i + 1) {
        result += text;
    }
    return result;
}

}  // namespace aih
