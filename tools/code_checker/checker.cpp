// Purpose: Implements checkedKind (which files are checked)
// and checkText (every rule for one file, sorted).
// Introduced in: Stage 0b (rebuild)
// Architecture: checkedKind follows the five steps of plan
// 3.4 in order. checkText runs the size rule first, then, for
// C++, cleaning, tokens and the four C++ rules.

#include "checker.h"

#include <algorithm>
#include <string>
#include <vector>

#include "architecture_rule.h"
#include "declaration_rule.h"
#include "function_rule.h"
#include "size_rules.h"
#include "style_rules.h"
#include "text_cleaner.h"

namespace aih {

namespace {

// Splits a forward-slash path into its parts.
std::vector<std::string> pathParts(const std::string& path) {
    std::vector<std::string> parts;
    std::string current;
    for (char c : path) {
        if (c == '/') {
            parts.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
        }
    }
    parts.push_back(current);
    return parts;
}

// True when text ends with suffix.
bool endsWith(const std::string& text, const std::string& suffix) {
    if (text.size() < suffix.size()) {
        return false;
    }
    return text.compare(text.size() - suffix.size(), suffix.size(), suffix) ==
           0;
}

// True when text starts with prefix.
bool startsWith(const std::string& text, const std::string& prefix) {
    if (text.size() < prefix.size()) {
        return false;
    }
    return text.compare(0, prefix.size(), prefix) == 0;
}

// True when any path part names a build folder.
bool hasBuildFolder(const std::vector<std::string>& parts) {
    for (const std::string& part : parts) {
        if (startsWith(part, "cmake-build-")) {
            return true;
        }
    }
    return false;
}

// True when two violations are exactly the same.
bool sameViolation(const Violation& a, const Violation& b) {
    if (a.line != b.line) {
        return false;
    }
    if (a.rule != b.rule) {
        return false;
    }
    return a.message == b.message;
}

// True when a sorts before b: line, then rule, then message.
bool violationLess(const Violation& a, const Violation& b) {
    if (a.line != b.line) {
        return a.line < b.line;
    }
    if (a.rule != b.rule) {
        return a.rule < b.rule;
    }
    return a.message < b.message;
}

// Sorts violations and removes exact duplicates.
std::vector<Violation> sortedUnique(std::vector<Violation> violations) {
    std::sort(violations.begin(), violations.end(), violationLess);
    std::vector<Violation> result;
    for (const Violation& violation : violations) {
        if (result.empty() || !sameViolation(result.back(), violation)) {
            result.push_back(violation);
        }
    }
    return result;
}

// Adds every violation from one rule to the growing list.
void addAll(std::vector<Violation>& target,
            const std::vector<Violation>& extra) {
    for (const Violation& violation : extra) {
        target.push_back(violation);
    }
}

}  // namespace

FileKind checkedKind(const std::string& relativePath) {
    std::vector<std::string> parts = pathParts(relativePath);
    if (hasBuildFolder(parts)) {
        return FileKind::None;
    }
    if (startsWith(relativePath, "tools/reference/")) {
        return FileKind::None;
    }
    if (endsWith(relativePath, ".ps1")) {
        if (parts.size() == 1 || parts[0] == "tools") {
            return FileKind::PowerShell;
        }
        return FileKind::None;
    }
    bool isCpp = endsWith(relativePath, ".h") || endsWith(relativePath, ".cpp");
    bool inTree =
        parts[0] == "src" || parts[0] == "tests" || parts[0] == "tools";
    if (isCpp && inTree && parts.size() > 1) {
        return FileKind::Cpp;
    }
    return FileKind::None;
}

std::vector<Violation> checkText(const std::string& path,
                                 const std::string& text, FileKind kind) {
    std::vector<Violation> violations;
    std::vector<std::string> lines = linesOf(text);
    addAll(violations, checkFileSize(lines));
    if (kind == FileKind::PowerShell) {
        addAll(violations, checkPowerShellFunctions(lines));
        return sortedUnique(violations);
    }
    CleanResult clean = cleanText(text);
    addAll(violations, checkRawStrings(clean.rawStringLines));
    std::vector<Token> tokens = tokenize(clean.cleaned);
    addAll(violations, checkFunctions(tokens));
    addAll(violations, checkBraces(tokens));
    addAll(violations, checkStatements(clean.cleaned));
    addAll(violations, checkDeclarations(clean.cleaned, tokens));
    addAll(violations, checkArchitecture(path, text, clean.cleaned));
    return sortedUnique(violations);
}

std::string formatViolation(const std::string& path,
                            const Violation& violation) {
    std::string line = std::to_string(violation.line);
    return path + ":" + line + ": " + violation.rule + " " + violation.message;
}

}  // namespace aih
