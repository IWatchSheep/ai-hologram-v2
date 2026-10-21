// Purpose: Implements readLines, reading a text file into
// lines and removing carriage returns, and parseInteger,
// the one integer test shared by the golden readers.
// Introduced in: Stage 1a (rebuild); parseInteger in 1b
// Architecture: std::ifstream reads the file one line at a
// time with std::getline, which removes the newline but keeps
// a carriage return left over from Windows line endings, so
// each line has any trailing carriage return removed here.

#include "golden_files.h"

#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

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

}  // namespace aih
