// Purpose: Declares the rule that agent code never includes
// experiment code (ARCH-INCLUDE, rule R7).
// Introduced in: Stage 0b (rebuild)
// Architecture: Only files under src/agent/ are checked. The
// include line is judged on cleaned text (so comments do not
// count) and its path on the original text.

#ifndef AI_HOLOGRAM_TOOLS_CODE_CHECKER_ARCHITECTURE_RULE_H
#define AI_HOLOGRAM_TOOLS_CODE_CHECKER_ARCHITECTURE_RULE_H

#include <string>
#include <vector>

#include "text_cleaner.h"

namespace aih {

// Reports ARCH-INCLUDE for includes of an experiment/ folder
// in a file whose path starts with src/agent/.
std::vector<Violation> checkArchitecture(const std::string& path,
                                         const std::string& text,
                                         const std::string& cleaned);

}  // namespace aih

#endif  // AI_HOLOGRAM_TOOLS_CODE_CHECKER_ARCHITECTURE_RULE_H
