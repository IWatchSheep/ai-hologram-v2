// Purpose: Declares the equivalence gate on supplied case and
// golden lines, and the exact number of golden cases it
// requires (plan 3.14).
// Introduced in: Stage 3g (rebuild), correction of plan v1.6
// Architecture: The gate lives apart from its tests so G4 and
// G5 can run it on the golden files while G6 and G7 run the
// same function on truncated inputs. Requiring the exact case
// count protects coverage: two files could match each other
// and still be truncated, so matching alone must not pass.

#ifndef AI_HOLOGRAM_TESTS_TRACE_GATE_H
#define AI_HOLOGRAM_TESTS_TRACE_GATE_H

#include <cstddef>
#include <string>
#include <vector>

namespace aih {

// The number of cases in the golden case file (plan 3.14).
constexpr std::size_t kGoldenCaseCount = 789;

// Runs the gate on the given lines: "" when every case
// matches its golden block, otherwise the first failure, in
// the order of plan 3.14. Nothing throws.
std::string runTraceGate(const std::vector<std::string>& caseLines,
                         const std::vector<std::string>& goldenLines,
                         bool observers, std::size_t requiredCases);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_TRACE_GATE_H
