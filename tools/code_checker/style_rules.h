// Purpose: Declares the braces, statements and raw-string
// style rules for C++ text.
// Introduced in: Stage 0b (rebuild)
// Architecture: The braces rule reads tokens, the statements
// rule reads cleaned lines, and raw-string lines come from
// the cleaning step in text_cleaner.

#ifndef AI_HOLOGRAM_TOOLS_CODE_CHECKER_STYLE_RULES_H
#define AI_HOLOGRAM_TOOLS_CODE_CHECKER_STYLE_RULES_H

#include <string>
#include <vector>

#include "text_cleaner.h"

namespace aih {

// Reports STYLE-BRACES where a body is not in braces.
std::vector<Violation> checkBraces(const std::vector<Token>& tokens);

// Reports STYLE-STATEMENTS for lines with several statements.
std::vector<Violation> checkStatements(const std::string& cleaned);

// Reports STYLE-RAW-STRING for each recorded raw string line.
std::vector<Violation> checkRawStrings(const std::vector<int>& rawLines);

}  // namespace aih

#endif  // AI_HOLOGRAM_TOOLS_CODE_CHECKER_STYLE_RULES_H
