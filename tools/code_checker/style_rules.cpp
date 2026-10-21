// Purpose: Implements the braces, statements and raw-string
// style rules for C++ text.
// Introduced in: Stage 0b (rebuild)
// Architecture: checkBraces reads the token list, looking at
// what follows each keyword's closing parenthesis (or the
// keyword itself for else and do). checkStatements counts
// semicolons outside parentheses across cleaned lines.

#include "style_rules.h"

#include <string>
#include <vector>

#include "text_cleaner.h"

namespace aih {

namespace {

// Builds one violation with the given line, rule and message.
Violation makeViolation(int line, const std::string& rule,
                        const std::string& message) {
    Violation violation;
    violation.line = line;
    violation.rule = rule;
    violation.message = message;
    return violation;
}

// True for keywords whose body must follow a (...) part.
bool isConditionKeyword(const std::string& word) {
    if (word == "if" || word == "for") {
        return true;
    }
    if (word == "while" || word == "switch") {
        return true;
    }
    return false;
}

// For if, an optional constexpr sits between keyword and '('.
// Returns the index of the '(' token, or -1 when there is none.
int conditionOpen(const std::vector<Token>& tokens, int k,
                  const std::string& word) {
    int size = static_cast<int>(tokens.size());
    int at = k;
    if (word == "if" && k + 1 < size && tokens[k + 1].value == "constexpr") {
        at = k + 1;
    }
    if (at + 1 >= size || tokens[at + 1].value != "(") {
        return -1;
    }
    return at + 1;
}

// Checks one if/for/while/switch token at index k.
void checkCondition(const std::vector<Token>& tokens, int k,
                    std::vector<Violation>& violations) {
    std::string word = tokens[k].value;
    int open = conditionOpen(tokens, k, word);
    if (open < 0) {
        return;
    }
    int close = findMatching(tokens, open, '(', ')');
    int size = static_cast<int>(tokens.size());
    if (close < 0 || close + 1 >= size) {
        return;
    }
    Token next = tokens[close + 1];
    bool isDoWhileEnd = false;
    if (word == "while" && next.value == ";" && k > 0) {
        isDoWhileEnd = tokens[k - 1].value == "}";
    }
    if (isDoWhileEnd) {
        return;
    }
    if (next.value != "{") {
        std::string message = "'" + word + "' body must be in braces";
        violations.push_back(
            makeViolation(tokens[k].line, "STYLE-BRACES", message));
    }
}

// Checks one else or do token at index k.
void checkElseOrDo(const std::vector<Token>& tokens, int k,
                   std::vector<Violation>& violations) {
    std::string word = tokens[k].value;
    int size = static_cast<int>(tokens.size());
    if (k + 1 >= size) {
        return;
    }
    std::string next = tokens[k + 1].value;
    if (word == "else" && next != "{" && next != "if") {
        violations.push_back(makeViolation(tokens[k].line, "STYLE-BRACES",
                                           "'else' body must be in braces"));
    }
    if (word == "do" && next != "{") {
        violations.push_back(makeViolation(tokens[k].line, "STYLE-BRACES",
                                           "'do' body must be in braces"));
    }
}

}  // namespace

std::vector<Violation> checkBraces(const std::vector<Token>& tokens) {
    std::vector<Violation> violations;
    int size = static_cast<int>(tokens.size());
    for (int k = 0; k < size; k = k + 1) {
        if (tokens[k].kind != TokenKind::Identifier) {
            continue;
        }
        std::string word = tokens[k].value;
        if (isConditionKeyword(word)) {
            checkCondition(tokens, k, violations);
        } else if (word == "else" || word == "do") {
            checkElseOrDo(tokens, k, violations);
        }
    }
    return violations;
}

std::vector<Violation> checkStatements(const std::string& cleaned) {
    std::vector<Violation> violations;
    std::vector<std::string> lines = linesOf(cleaned);
    int depth = 0;
    int lineNumber = 0;
    for (const std::string& line : lines) {
        lineNumber = lineNumber + 1;
        int count = 0;
        for (char c : line) {
            if (c == '(') {
                depth = depth + 1;
            } else if (c == ')') {
                depth = depth - 1;
                if (depth < 0) {
                    depth = 0;
                }
            } else if (c == ';' && depth == 0) {
                count = count + 1;
            }
        }
        if (count > 1) {
            violations.push_back(
                makeViolation(lineNumber, "STYLE-STATEMENTS",
                              "more than one statement on one line"));
        }
    }
    return violations;
}

std::vector<Violation> checkRawStrings(const std::vector<int>& rawLines) {
    std::vector<Violation> violations;
    for (int line : rawLines) {
        violations.push_back(makeViolation(
            line, "STYLE-RAW-STRING", "raw string literals are not allowed"));
    }
    return violations;
}

}  // namespace aih
