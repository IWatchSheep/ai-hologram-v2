// Purpose: Implements ARCH-INCLUDE: agent code must not
// include experiment code (rule R7).
// Introduced in: Stage 0b (rebuild)
// Architecture: A line counts as an include when the cleaned
// line (comments blanked) starts with #include. Its path is
// read from the original line and checked for an experiment/
// folder after backslashes become forward slashes.

#include "architecture_rule.h"

#include <string>
#include <vector>

#include "text_cleaner.h"

namespace aih {

namespace {

// Replaces every backslash in text with a forward slash.
std::string withForwardSlashes(const std::string& text) {
    std::string result = text;
    for (char& c : result) {
        if (c == '\\') {
            c = '/';
        }
    }
    return result;
}

// True when the cleaned line is an #include line: optional
// spaces, '#', optional spaces, the word include, then a
// character that is not a word character (or the line ends).
bool isIncludeLine(const std::string& line) {
    int size = static_cast<int>(line.size());
    int i = 0;
    while (i < size && isSpaceChar(line[i])) {
        i = i + 1;
    }
    if (i >= size || line[i] != '#') {
        return false;
    }
    i = i + 1;
    while (i < size && isSpaceChar(line[i])) {
        i = i + 1;
    }
    std::string word = "include";
    for (char expected : word) {
        if (i >= size || line[i] != expected) {
            return false;
        }
        i = i + 1;
    }
    return i >= size || !isWordChar(line[i]);
}

// Extracts the path from the first "..." or <...> on the
// original line, or returns an empty string when there is none.
std::string includePathOf(const std::string& line) {
    int size = static_cast<int>(line.size());
    int start = -1;
    for (int i = 0; i < size; i = i + 1) {
        if (line[i] == '"' || line[i] == '<') {
            start = i;
            break;
        }
    }
    if (start < 0) {
        return "";
    }
    for (int i = start + 1; i < size; i = i + 1) {
        if (line[i] == '"' || line[i] == '>') {
            return line.substr(start + 1, i - start - 1);
        }
    }
    return "";
}

// True when experiment/ appears as a folder in the path: at
// the start or right after a '/'.
bool hasExperimentFolder(const std::string& includePath) {
    std::string path = withForwardSlashes(includePath);
    std::string folder = "experiment/";
    if (path.compare(0, folder.size(), folder) == 0) {
        return true;
    }
    std::string inside = "/" + folder;
    return path.find(inside) != std::string::npos;
}

}  // namespace

std::vector<Violation> checkArchitecture(const std::string& path,
                                         const std::string& text,
                                         const std::string& cleaned) {
    std::vector<Violation> violations;
    std::string normalized = withForwardSlashes(path);
    if (normalized.compare(0, 10, "src/agent/") != 0) {
        return violations;
    }
    std::vector<std::string> lines = linesOf(text);
    std::vector<std::string> cleanedLines = linesOf(cleaned);
    int count = static_cast<int>(lines.size());
    for (int i = 0; i < count; i = i + 1) {
        if (!isIncludeLine(cleanedLines[i])) {
            continue;
        }
        if (hasExperimentFolder(includePathOf(lines[i]))) {
            Violation violation;
            violation.line = i + 1;
            violation.rule = "ARCH-INCLUDE";
            violation.message = "agent code must not include experiment code";
            violations.push_back(violation);
        }
    }
    return violations;
}

}  // namespace aih
