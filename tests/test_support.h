// Purpose: Declares the small test helper shared by every test
// group: a counter and the check() function that records results.
// Introduced in: Stage 0a (rebuild)
// Architecture: TestCounter carries the counts, the verbose flag
// and where output goes, so tests can capture output in a stream
// instead of printing to the screen (used by TS1 to TS3).

#ifndef AI_HOLOGRAM_TESTS_TEST_SUPPORT_H
#define AI_HOLOGRAM_TESTS_TEST_SUPPORT_H

#include <iostream>
#include <ostream>
#include <string>

namespace aih {

// Counts checks and failures for one run of the test program.
// The out pointer lets a test redirect output to its own stream.
struct TestCounter {
    int checks = 0;
    int failures = 0;
    bool verbose = false;
    std::ostream* out = &std::cout;
};

// Records one check. A failure always prints a FAIL line, a
// success prints a PASS line only when verbose is true.
void check(TestCounter& counter, bool condition,
           const std::string& description);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_TEST_SUPPORT_H
