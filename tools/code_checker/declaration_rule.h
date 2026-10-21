// Purpose: Declares the rule that one statement declares at
// most one thing (STYLE-DECLARATIONS).
// Introduced in: Stage 0b (rebuild)
// Architecture: The cleaned text is scanned character by
// character, tracking parentheses, brackets, brace nesting
// and template angle brackets, as the reference does.

#ifndef AI_HOLOGRAM_TOOLS_CODE_CHECKER_DECLARATION_RULE_H
#define AI_HOLOGRAM_TOOLS_CODE_CHECKER_DECLARATION_RULE_H

#include <string>
#include <vector>

#include "text_cleaner.h"

namespace aih {

// Reports STYLE-DECLARATIONS at the first comma of a statement
// that declares more than one thing in one statement.
std::vector<Violation> checkDeclarations(const std::string& cleaned,
                                         const std::vector<Token>& tokens);

}  // namespace aih

#endif  // AI_HOLOGRAM_TOOLS_CODE_CHECKER_DECLARATION_RULE_H
