// Purpose: Declares the hand-drawn test layouts of plan
// Section 6.1 and the helpers that vary them.
// Introduced in: Stage 1a (rebuild)
// Architecture: Every test group reads the layouts and the
// withLine and at helpers from here, so there is one copy of
// each (rule R4). The layouts are returned by functions, so
// no test can accidentally change another test's layout.

#ifndef AI_HOLOGRAM_TESTS_TEST_LAYOUTS_H
#define AI_HOLOGRAM_TESTS_TEST_LAYOUTS_H

#include <string>
#include <vector>

#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// The DEMO layout: agent (3, 3), object (5, 3), destination
// (8, 3), with two wall cells at (4, 6) and (5, 6).
std::vector<std::string> demoLayout();

// The STAR layout: the object stands on the destination (*).
std::vector<std::string> starLayout();

// The PLUS layout: the agent stands on the destination (+).
std::vector<std::string> plusLayout();

// The OPEN layout: agent (4, 4), object (7, 7), destination
// (1, 8), and no inner walls.
std::vector<std::string> openLayout();

// Returns a copy of layout with one line replaced by text.
std::vector<std::string> withLine(const std::vector<std::string>& layout,
                                  int index, const std::string& text);

// Returns the parsed layout with the agent and the object
// moved to the given positions, written directly into the
// state. The layout itself must parse successfully.
WorldState at(const std::vector<std::string>& layout, Position agent,
              Position object);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_TEST_LAYOUTS_H
