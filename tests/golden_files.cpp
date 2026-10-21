// Purpose: Implements readLines, reading a text file into
// lines and removing carriage returns, parseInteger, the
// one integer test shared by the golden readers, and
// splitWords and startsWith, shared by both readers too,
// and actionFromName, the one action-name lookup.
// Introduced in: Stage 1a (rebuild); parseInteger in 1b;
// splitWords and startsWith in Stage 1c; actionFromName in 2c
// Architecture: std::ifstream reads the file one line at a
// time with std::getline, which removes the newline but keeps
// a carriage return left over from Windows line endings, so
// each line has any trailing carriage return removed here.
// The text helpers moved here in Stage 1c so the two golden
// readers share one copy of each (rule R4).

#include "golden_files.h"

#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

#include "world/action.h"

namespace aih {

std::vector<std::string> readLines(const std::string& path) {
    std::vector<std::string> lines;
    std::ifstream input(path);
    if (!input.is_open()) {
        return lines;
    }
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty()) {
            if (line.back() == '\r') {
                line.pop_back();
            }
        }
        lines.push_back(line);
    }
    return lines;
}

bool parseInteger(const std::string& text, int& value) {
    value = 0;
    if (text.empty()) {
        return false;
    }
    std::size_t index = 0;
    bool negative = false;
    if (text[0] == '-') {
        negative = true;
        index = 1;
        if (text.size() == 1) {
            return false;
        }
    }
    // At most 9 digits may follow the optional '-', so the
    // result below can never overflow an int (plan 6.7, v1.3).
    if (text.size() - index > 9) {
        return false;
    }
    int result = 0;
    while (index < text.size()) {
        const char character = text[index];
        if (character < '0' || character > '9') {
            return false;
        }
        result = result * 10 + (character - '0');
        index = index + 1;
    }
    if (negative) {
        value = -result;
    } else {
        value = result;
    }
    return true;
}

std::vector<std::string> splitWords(const std::string& text) {
    std::vector<std::string> words;
    std::string word;
    for (const char character : text) {
        if (character == ' ') {
            words.push_back(word);
            word.clear();
        } else {
            word.push_back(character);
        }
    }
    words.push_back(word);
    return words;
}

bool startsWith(const std::string& line, const std::string& prefix) {
    if (line.size() < prefix.size()) {
        return false;
    }
    return line.compare(0, prefix.size(), prefix) == 0;
}

bool actionFromName(const std::string& name, Action& action) {
    const Action actions[] = {Action::North, Action::East, Action::South,
                              Action::West};
    for (const Action candidate : actions) {
        if (actionName(candidate) == name) {
            action = candidate;
            return true;
        }
    }
    return false;
}

}  // namespace aih
