// Purpose: Declares readLines, the one helper that reads a
// text file into lines for the golden tests, parseInteger,
// the one integer test for the golden readers, and
// splitWords and startsWith, the one word-splitting and
// prefix tests shared by both golden readers, and
// actionFromName, the one action-name lookup.
// Introduced in: Stage 1a (rebuild); parseInteger in 1b;
// splitWords and startsWith in Stage 1c; actionFromName in 2c
// Architecture: File reading and these small text helpers
// live here only (rule R4), so both golden readers of this
// stage share exactly the same code instead of two copies.

#ifndef AI_HOLOGRAM_TESTS_GOLDEN_FILES_H
#define AI_HOLOGRAM_TESTS_GOLDEN_FILES_H

#include <string>
#include <vector>

#include "world/action.h"

namespace aih {

// Reads the file at path into its lines, removing any carriage
// return characters, so files with Windows or Unix line
// endings read the same way. A file that cannot be opened
// gives an empty list, which the golden tests treat as a
// failure with a clear message, never as a pass.
std::vector<std::string> readLines(const std::string& path);

// The one integer test (rule R4), shared by both golden
// readers: an optional '-' and then 1 to 9 digits only. At
// most 9 digits, so an int can never overflow. On success
// it returns true and sets value; otherwise it returns
// false and sets value to 0.
bool parseInteger(const std::string& text, int& value);

// Splits text at every single space, keeping empty words,
// exactly like the reference's split(' ').
std::vector<std::string> splitWords(const std::string& text);

// Returns whether line starts with the whole prefix.
bool startsWith(const std::string& line, const std::string& prefix);

// Turns one of the four action names into its Action, by
// comparing with actionName, the one place names are given.
// On success it returns true and sets action; otherwise it
// returns false.
bool actionFromName(const std::string& name, Action& action);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_GOLDEN_FILES_H
