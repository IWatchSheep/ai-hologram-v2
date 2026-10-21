// Purpose: Declares readLines, the one helper that reads a
// text file into lines for the golden tests, and
// parseInteger, the one integer test for the golden readers.
// Introduced in: Stage 1a (rebuild); parseInteger in 1b
// Architecture: File reading lives here only (rule R4), so
// the golden tests of this stage and Stage 1b share exactly
// the same reading and line-ending handling.

#ifndef AI_HOLOGRAM_TESTS_GOLDEN_FILES_H
#define AI_HOLOGRAM_TESTS_GOLDEN_FILES_H

#include <string>
#include <vector>

namespace aih {

// Reads the file at path into its lines, removing any carriage
// return characters, so files with Windows or Unix line
// endings read the same way. A file that cannot be opened
// gives an empty list, which the golden tests treat as a
// failure with a clear message, never as a pass.
std::vector<std::string> readLines(const std::string& path);

// The one integer test (rule R4), shared by both golden
// readers: an optional '-' and then digits only. On success
// it returns true and sets value; otherwise it returns
// false and sets value to 0.
bool parseInteger(const std::string& text, int& value);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_GOLDEN_FILES_H
