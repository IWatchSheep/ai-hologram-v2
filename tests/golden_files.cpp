// Purpose: Implements readLines, reading a text file into
// lines and removing carriage returns.
// Introduced in: Stage 1a (rebuild)
// Architecture: std::ifstream reads the file one line at a
// time with std::getline, which removes the newline but keeps
// a carriage return left over from Windows line endings, so
// each line has any trailing carriage return removed here.

#include "golden_files.h"

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

}  // namespace aih
