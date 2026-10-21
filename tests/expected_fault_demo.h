// Purpose: Declares the expected 68 lines of the Stage 4
// demonstration (lines 103-170 of the expected program
// output).
// Introduced in: Stage 4f (rebuild)
// Architecture: One copy of the expected lines lives here
// (rule R4), so FD1 compares runFaultDemo with exactly
// this text. The lines are copied exactly from
// tools/reference/stage4_rebuild/
// expected_program_output.txt, lines 103-170.

#ifndef AI_HOLOGRAM_TESTS_EXPECTED_FAULT_DEMO_H
#define AI_HOLOGRAM_TESTS_EXPECTED_FAULT_DEMO_H

#include <string>
#include <vector>

namespace aih {

// The 68 expected demonstration lines, in order.
std::vector<std::string> expectedFaultDemoLines();

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_EXPECTED_FAULT_DEMO_H
