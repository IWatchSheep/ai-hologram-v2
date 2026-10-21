// Purpose: Declares the line and word readers that
// readTraceCases uses, one copy of each (plan 3.14, R4).
// Introduced in: Stage 3g (rebuild)
// Architecture: readTraceCases in trace_cases.cpp walks a
// case stage by stage; these helpers read one stage's words
// each. Every word is read only after the word count is
// checked, so a malformed line is reported, never read out
// of range, and nothing throws.

#ifndef AI_HOLOGRAM_TESTS_TRACE_CASE_PARTS_H
#define AI_HOLOGRAM_TESTS_TRACE_CASE_PARTS_H

#include <optional>
#include <string>
#include <vector>

#include "agent/agent_types.h"
#include "trace_cases.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// A non-negative integer of at most 9 digits, or nothing.
std::optional<int> traceInteger(const std::string& text);

// Reads a settings line's words into settings; false when
// the words are not "settings" and four integers >= 0.
bool traceSettingsFromWords(const std::vector<std::string>& words,
                            AgentSettings& settings);

// Parses the 10 world rows starting at firstRow. On failure
// problem is the full "case <n>: ..." text of the reference.
bool traceWorldRows(const std::vector<std::string>& lines, int firstRow,
                    int caseNumber, const std::string& what, WorldState& world,
                    std::string& problem);

// True when the words are "model" and one catalogue form.
bool traceModelWordsOk(const std::vector<std::string>& words);

// Reads a "notdead" line's cells; false when malformed.
bool traceNotDeadFromWords(const std::vector<std::string>& words,
                           std::vector<Position>& cells);

// Applies one "seed ..." line to the case, with the
// duplicate rules for v1, committed and plan; false when
// the line is malformed.
bool applyTraceSeed(TraceCase& traceCase,
                    const std::vector<std::string>& words);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_TRACE_CASE_PARTS_H
