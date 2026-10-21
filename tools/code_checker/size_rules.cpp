// Purpose: Implements the file-size rule and the PowerShell
// function-size rule.
// Introduced in: Stage 0b (rebuild)
// Architecture: A PowerShell function starts at a line that
// begins with the word function and ends where its braces
// balance, exactly as the reference measures it.

#include "size_rules.h"

#include <string>
#include <vector>

#include "text_cleaner.h"

namespace aih {

namespace {

// True for the characters a PowerShell name may contain:
// a word character (letter, digit or '_') or '-'.
bool isNameChar(char c) {
    return isWordChar(c) || c == '-';
}

// Lowercases one letter; other characters stay as they are.
char lowerChar(char c) {
    if (c >= 'A' && c <= 'Z') {
        return static_cast<char>(c - 'A' + 'a');
    }
    return c;
}

// True when line begins (after blanks) with "function", then
// blanks, then a name: the reference pattern function\s+[\w-]+.
bool isFunctionStart(const std::string& line) {
    int size = static_cast<int>(line.size());
    int i = 0;
    while (i < size && isSpaceChar(line[i])) {
        i = i + 1;
    }
    std::string word = "function";
    for (char expected : word) {
        if (i >= size || lowerChar(line[i]) != expected) {
            return false;
        }
        i = i + 1;
    }
    if (i >= size || !isSpaceChar(line[i])) {
        return false;
    }
    while (i < size && isSpaceChar(line[i])) {
        i = i + 1;
    }
    return i < size && isNameChar(line[i]);
}

// Counts { minus } on one line, noting if any brace was seen.
int braceBalance(const std::string& line, bool& seen) {
    int balance = 0;
    for (char c : line) {
        if (c == '{') {
            balance = balance + 1;
            seen = true;
        }
        if (c == '}') {
            balance = balance - 1;
            seen = true;
        }
    }
    return balance;
}

// Measures one PowerShell function starting at line start.
void measureFunction(const std::vector<std::string>& lines, int start,
                     std::vector<Violation>& violations) {
    int depth = 0;
    bool started = false;
    int count = static_cast<int>(lines.size());
    for (int j = start; j < count; j = j + 1) {
        depth = depth + braceBalance(lines[j], started);
        if (started && depth == 0) {
            int length = j - start + 1;
            if (length > kMaxFunctionLines) {
                Violation violation;
                violation.line = start + 1;
                violation.rule = "SIZE-FUNCTION";
                violation.message = "function is " + std::to_string(length) +
                                    " lines (limit " +
                                    std::to_string(kMaxFunctionLines) + ")";
                violations.push_back(violation);
            }
            return;
        }
    }
}

}  // namespace

std::vector<Violation> checkFileSize(const std::vector<std::string>& lines) {
    std::vector<Violation> violations;
    int count = static_cast<int>(lines.size());
    if (count > kMaxFileLines) {
        Violation violation;
        violation.line = 1;
        violation.rule = "SIZE-FILE";
        violation.message = "file is " + std::to_string(count) +
                            " lines (limit " + std::to_string(kMaxFileLines) +
                            ")";
        violations.push_back(violation);
    }
    return violations;
}

std::vector<Violation> checkPowerShellFunctions(
    const std::vector<std::string>& lines) {
    std::vector<Violation> violations;
    int count = static_cast<int>(lines.size());
    for (int i = 0; i < count; i = i + 1) {
        if (isFunctionStart(lines[i])) {
            measureFunction(lines, i, violations);
        }
    }
    return violations;
}

}  // namespace aih
