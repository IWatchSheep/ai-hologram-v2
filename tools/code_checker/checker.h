// Purpose: Declares which files are checked and the function
// that runs every rule on one file's text.
// Introduced in: Stage 0b (rebuild)
// Architecture: checkedKind is the one place that decides
// whether a path is checked and how (plan 3.4, D-058), and
// checkText combines the rules and sorts their violations.

#ifndef AI_HOLOGRAM_TOOLS_CODE_CHECKER_CHECKER_H
#define AI_HOLOGRAM_TOOLS_CODE_CHECKER_CHECKER_H

#include <string>
#include <vector>

#include "text_cleaner.h"

namespace aih {

// How a file is checked: not at all, as C++, or as PowerShell.
enum class FileKind { None, Cpp, PowerShell };

// Returns how the file at this root-relative path (forward
// slashes) is checked, following the five steps in plan 3.4.
FileKind checkedKind(const std::string& relativePath);

// Runs every rule for the kind on text and returns the
// violations sorted by line, then rule, then message.
std::vector<Violation> checkText(const std::string& path,
                                 const std::string& text, FileKind kind);

// Formats one violation as <path>:<line>: <RULE> <message>.
std::string formatViolation(const std::string& path,
                            const Violation& violation);

}  // namespace aih

#endif  // AI_HOLOGRAM_TOOLS_CODE_CHECKER_CHECKER_H
