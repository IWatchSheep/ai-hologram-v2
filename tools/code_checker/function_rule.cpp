// Purpose: Implements the C++ function-size rule: function
// and lambda bodies over 40 lines are reported.
// Introduced in: Stage 0b (rebuild)
// Architecture: Tokens are scanned with a stack of scopes.
// For each '{', the tokens since the previous ';', '{' or '}'
// (the head) decide what kind of scope it opens (plan 3.4).

#include "function_rule.h"

#include <string>
#include <vector>

#include "size_rules.h"
#include "text_cleaner.h"

namespace aih {

namespace {

// One open scope: its kind and the line of its '{'.
struct Scope {
    std::string kind;
    int openLine = 1;
};

// True when a token in the head has the given text.
bool headHas(const std::vector<Token>& head, const std::string& value) {
    for (const Token& token : head) {
        if (token.value == value) {
            return true;
        }
    }
    return false;
}

// Only specifiers from start: no '(', ')', '[' ']' or '=',
// except noexcept and decltype may take one (...) group each.
bool onlySpecifiers(const std::vector<Token>& tokens, int start) {
    int size = static_cast<int>(tokens.size());
    int k = start;
    while (k < size) {
        std::string word = tokens[k].value;
        bool grouped = word == "noexcept" || word == "decltype";
        if (grouped && k + 1 < size && tokens[k + 1].value == "(") {
            int close = findMatching(tokens, k + 1, '(', ')');
            if (close < 0) {
                return false;
            }
            k = close + 1;
            continue;
        }
        if (word == "(" || word == ")" || word == "=" || word == "[" ||
            word == "]") {
            return false;
        }
        k = k + 1;
    }
    return true;
}

// Checks one possible lambda introducer at head index a.
bool lambdaAt(const std::vector<Token>& head, int a) {
    int size = static_cast<int>(head.size());
    if (a + 1 < size && head[a + 1].value == "[") {
        return false;
    }
    const Token* previous = a > 0 ? &head[a - 1] : nullptr;
    if (!isLambdaIntroducer(previous)) {
        return false;
    }
    int close = findMatching(head, a, '[', ']');
    if (close < 0) {
        return false;
    }
    int rest = close + 1;
    if (rest < size && head[rest].value == "(") {
        int after = findMatching(head, rest, '(', ')');
        if (after < 0) {
            return false;
        }
        rest = after + 1;
    }
    return onlySpecifiers(head, rest);
}

// True when the head holds a lambda introducer followed by
// nothing, or by a parameter list and only specifiers.
bool lambdaInHead(const std::vector<Token>& head) {
    int size = static_cast<int>(head.size());
    for (int a = size - 1; a >= 0; a = a - 1) {
        if (head[a].value == "[" && lambdaAt(head, a)) {
            return true;
        }
    }
    return false;
}

// True when the head holds class, struct, union or enum.
bool headHasClassWord(const std::vector<Token>& head) {
    return headHas(head, "class") || headHas(head, "struct") ||
           headHas(head, "union") || headHas(head, "enum");
}

// An '=' outside parentheses makes '{' an initializer.
bool headHasOuterEquals(const std::vector<Token>& head) {
    int depth = 0;
    bool hasEquals = false;
    for (const Token& token : head) {
        if (token.value == "(") {
            depth = depth + 1;
        } else if (token.value == ")") {
            depth = depth - 1;
        } else if (token.value == "=" && depth == 0) {
            hasEquals = true;
        }
    }
    return hasEquals;
}

// Decides what kind of scope a '{' opens, in plan order.
std::string scopeKind(const std::vector<Token>& head,
                      const std::vector<Scope>& stack) {
    std::string outer = stack.empty() ? "namespace" : stack.back().kind;
    if (lambdaInHead(head)) {
        return "lambda";
    }
    if (headHas(head, "namespace")) {
        return "namespace";
    }
    if (outer == "function" || outer == "block" || outer == "lambda") {
        return "block";
    }
    if (headHasClassWord(head) && !headHas(head, ")")) {
        return "class";
    }
    if (headHas(head, ")")) {
        bool init = headHasOuterEquals(head) && !headHas(head, "operator");
        return init ? "block" : "function";
    }
    return "block";
}

// Reports a finished function or lambda body if it is long.
void closeScope(const Scope& scope, int closeLine,
                std::vector<Violation>& violations) {
    if (scope.kind != "function" && scope.kind != "lambda") {
        return;
    }
    int length = closeLine - scope.openLine + 1;
    if (length > kMaxFunctionLines) {
        std::string message = "function is " + std::to_string(length) +
                              " lines (limit " +
                              std::to_string(kMaxFunctionLines) + ")";
        violations.push_back({scope.openLine, "SIZE-FUNCTION", message});
    }
}

}  // namespace

bool isLambdaIntroducer(const Token* previous) {
    if (previous == nullptr || previous->value == "return") {
        return true;
    }
    if (previous->kind == TokenKind::Identifier) {
        return false;
    }
    if (previous->kind == TokenKind::Number) {
        return false;
    }
    return previous->value != ")" && previous->value != "]" &&
           previous->value != "[";
}

std::vector<Violation> checkFunctions(const std::vector<Token>& tokens) {
    std::vector<Violation> violations;
    std::vector<Scope> stack;
    std::vector<Token> head;
    for (const Token& token : tokens) {
        bool isPunct = token.kind == TokenKind::Punctuation;
        if (isPunct && token.value == "{") {
            stack.push_back({scopeKind(head, stack), token.line});
            head.clear();
        } else if (isPunct && token.value == "}") {
            if (!stack.empty()) {
                Scope scope = stack.back();
                stack.pop_back();
                closeScope(scope, token.line, violations);
            }
            head.clear();
        } else if (isPunct && token.value == ";") {
            head.clear();
        } else {
            head.push_back(token);
        }
    }
    return violations;
}

}  // namespace aih
