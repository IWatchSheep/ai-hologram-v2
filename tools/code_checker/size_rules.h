// Purpose: Declares the file-size rule and the PowerShell
// function-size rule, plus the limits both kinds share.
// Introduced in: Stage 0b (rebuild)
// Architecture: The limits live here so the C++ function rule
// and the PowerShell rule use the one constant (plan 3.4).

#ifndef AI_HOLOGRAM_TOOLS_CODE_CHECKER_SIZE_RULES_H
#define AI_HOLOGRAM_TOOLS_CODE_CHECKER_SIZE_RULES_H

#include <string>
#include <vector>

#include "text_cleaner.h"

namespace aih {

// A file may have at most this many lines (rule R1).
const int kMaxFileLines = 200;

// A function or lambda body may span at most this many lines.
const int kMaxFunctionLines = 40;

// Reports SIZE-FILE at line 1 when there are too many lines.
std::vector<Violation> checkFileSize(const std::vector<std::string>& lines);

// Reports SIZE-FUNCTION for long PowerShell functions.
std::vector<Violation> checkPowerShellFunctions(
    const std::vector<std::string>& lines);

}  // namespace aih

#endif  // AI_HOLOGRAM_TOOLS_CODE_CHECKER_SIZE_RULES_H
