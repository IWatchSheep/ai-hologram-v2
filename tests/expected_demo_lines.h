// Purpose: Declares the expected run lines of the two C1
// demonstrations (checks.py C1_1 and C1_2).
// Introduced in: Stage 3e (rebuild)
// Architecture: One copy of each list lives here (rule
// R4), shared by C1-1, C1-2 and OB6 in 3e and by AD1 in 3f,
// so the demonstrations' expected text is written once.

#ifndef AI_HOLOGRAM_TESTS_EXPECTED_DEMO_LINES_H
#define AI_HOLOGRAM_TESTS_EXPECTED_DEMO_LINES_H

#include <string>
#include <vector>

namespace aih {

// The run lines of demo 1 (correct belief): checks.py C1_1.
std::vector<std::string> c1Demo1Lines();

// The run lines of demo 2 (a wall the belief lacks at
// (4, 3)): checks.py C1_2, with the active-test line at
// index 8.
std::vector<std::string> c1Demo2Lines();

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_EXPECTED_DEMO_LINES_H
