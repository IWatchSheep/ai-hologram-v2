// Purpose: Tests for renderWorld (RT1 to RT3, plan
// Section 6.3): rendering a parsed layout gives it back.
// Introduced in: Stage 1a (rebuild)
// Architecture: Each check parses one layout of test_layouts
// and compares the rendered lines with the layout exactly.

#include <string>
#include <vector>

#include "test_layouts.h"
#include "test_suites.h"
#include "test_support.h"
#include "world/world_text.h"

namespace aih {
namespace {

// Parses the layout and renders it again, returning the
// rendered lines, or no lines when parsing fails.
std::vector<std::string> roundTrip(const std::vector<std::string>& layout) {
    const ParseResult result = parseWorld(layout);
    if (!result.world.has_value()) {
        return {};
    }
    return renderWorld(result.world.value());
}

}  // namespace

void runWorldRenderTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- World render ---\n";
    }
    check(counter, roundTrip(demoLayout()) == demoLayout(),
          "RT1: rendering parsed DEMO gives DEMO exactly");
    check(counter, roundTrip(starLayout()) == starLayout(),
          "RT2: rendering parsed STAR gives STAR exactly");
    check(counter, roundTrip(plusLayout()) == plusLayout(),
          "RT3: rendering parsed PLUS gives PLUS exactly");
}

}  // namespace aih
