// Purpose: Declares the equivalence gate, part 2, on
// supplied case and golden lines, and the exact number
// of golden fault cases it requires (plan 3.15).
// Introduced in: Stage 4g (rebuild)
// Architecture: The gate replays the 563 fault runs
// recorded from the original C++ and compares every line,
// which proves the rebuild does exactly what the original
// code did. The golden files come from that original code
// and are never edited. Requiring the exact case count
// protects coverage: two files could match each other and
// still be truncated, so matching alone must not pass;
// G9 and G10 exist to prove this count is enforced.

#ifndef AI_HOLOGRAM_TESTS_FAULT_GATE_H
#define AI_HOLOGRAM_TESTS_FAULT_GATE_H

#include <cstddef>
#include <string>
#include <vector>

namespace aih {

// The number of cases in the golden fault case file.
constexpr std::size_t kGoldenFaultCaseCount = 563;

// Runs the gate on the given lines: "" when every case
// matches its golden block, otherwise the first failure,
// in the order of plan 3.15 ("The gate"). Nothing throws.
std::string runFaultGate(const std::vector<std::string>& caseLines,
                         const std::vector<std::string>& goldenLines,
                         std::size_t requiredCases);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_FAULT_GATE_H
