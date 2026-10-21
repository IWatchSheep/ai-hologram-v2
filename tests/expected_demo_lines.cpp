// Purpose: Defines the expected run lines of the two C1
// demonstrations, exactly as checks.py writes them.
// Introduced in: Stage 3e (rebuild)
// Architecture: Plain expected text, one line per entry, in
// run order: the agent's log lines, then the four result
// lines. No agent code is called here.

#include "expected_demo_lines.h"

#include <string>
#include <vector>

namespace aih {

std::vector<std::string> c1Demo1Lines() {
    return {"Plan: search found a plan of 4 actions",
            "Step 1: East | predicted Moved, observed Moved | match",
            "Step 2: East | predicted Pushed, observed Pushed | match",
            "Step 3: East | predicted Pushed, observed Pushed | match",
            "Step 4: East | predicted Pushed, observed Pushed | match",
            "Result: Success after 4 world steps, 0 safe stops",
            "Mismatches: 0",
            "Corrections: 0 (0 verified)",
            "Model evaluations used: 19"};
}

std::vector<std::string> c1Demo2Lines() {
    return {"Plan: search found a plan of 4 actions",
            "Step 1: East | predicted Moved, observed Blocked | MISMATCH",
            "  - investigation: wall at (4, 3) believed free, recorded "
            "blocked (source S2)",
            "  - correction 1: wall at (4, 3) set to blocked: CONSISTENT",
            "Plan: fallback toward the best state found (10 actions)",
            "Step 2: North | predicted Moved, observed Moved | match",
            "Step 3: East | predicted Moved, observed Moved | match",
            "Step 4: East | predicted Moved, observed Moved | match",
            "Plan: active test of correction 1 (2 actions)",
            "Step 5: West | predicted Moved, observed Moved | match",
            "Step 6: South | predicted Blocked, observed Blocked | match",
            "  - correction 1: VERIFIED",
            "Plan: search found a plan of 12 actions",
            "Step 7: East | predicted Moved, observed Moved | match",
            "Step 8: South | predicted Pushed, observed Pushed | match",
            "Step 9: South | predicted Pushed, observed Pushed | match",
            "Step 10: West | predicted Moved, observed Moved | match",
            "Step 11: South | predicted Moved, observed Moved | match",
            "Step 12: East | predicted Pushed, observed Pushed | match",
            "Step 13: East | predicted Pushed, observed Pushed | match",
            "Step 14: East | predicted Pushed, observed Pushed | match",
            "Step 15: South | predicted Moved, observed Moved | match",
            "Step 16: East | predicted Moved, observed Moved | match",
            "Step 17: North | predicted Pushed, observed Pushed | match",
            "Step 18: North | predicted Pushed, observed Pushed | match",
            "Result: Success after 18 world steps, 0 safe stops",
            "Mismatches: 1",
            "Corrections: 1 (1 verified)",
            "Model evaluations used: 414"};
}

}  // namespace aih
