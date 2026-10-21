// Purpose: Declares the C++ function-size rule and the one
// lambda-introducer test that other rules also use.
// Introduced in: Stage 0b (rebuild)
// Architecture: A stack of scopes decides which braces open a
// function or lambda body; both share the 40-line limit.

#ifndef AI_HOLOGRAM_TOOLS_CODE_CHECKER_FUNCTION_RULE_H
#define AI_HOLOGRAM_TOOLS_CODE_CHECKER_FUNCTION_RULE_H

#include <vector>

#include "text_cleaner.h"

namespace aih {

// True when a '[' token starts a lambda introducer. previous
// is the token before it, or nullptr when there is none. A
// '[' after an identifier (except return), a number, ')', ']'
// or '[' is a subscript, a declarator or an attribute instead.
bool isLambdaIntroducer(const Token* previous);

// Reports SIZE-FUNCTION for function and lambda bodies whose
// opening '{' to closing '}' span is over the limit.
std::vector<Violation> checkFunctions(const std::vector<Token>& tokens);

}  // namespace aih

#endif  // AI_HOLOGRAM_TOOLS_CODE_CHECKER_FUNCTION_RULE_H
