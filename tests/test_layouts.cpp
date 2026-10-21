// Purpose: Defines the hand-drawn test layouts of plan
// Section 6.1 and the helpers that vary them.
// Introduced in: Stage 1a (rebuild)
// Architecture: The four layouts are written out in full so a
// reader can compare them with the plan line by line. Row 0
// is the first text line and y grows downwards, as in the
// world itself.

#include "test_layouts.h"

#include <cstddef>
#include <string>
#include <vector>

#include "world/position.h"
#include "world/world_state.h"
#include "world/world_text.h"

namespace aih {

std::vector<std::string> starLayout() {
    return {"##########", "#........#", "#........#", "#..A....*#",
            "#........#", "#........#", "#...##...#", "#........#",
            "#........#", "##########"};
}

std::vector<std::string> plusLayout() {
    return {"##########", "#........#", "#........#", "#..O....+#",
            "#........#", "#........#", "#...##...#", "#........#",
            "#........#", "##########"};
}

std::vector<std::string> openLayout() {
    return {"##########", "#........#", "#........#", "#........#",
            "#...A....#", "#........#", "#........#", "#......O.#",
            "#D.......#", "##########"};
}

std::vector<std::string> withLine(const std::vector<std::string>& layout,
                                  int index, const std::string& text) {
    std::vector<std::string> copy = layout;
    copy[static_cast<std::size_t>(index)] = text;
    return copy;
}

WorldState at(const std::vector<std::string>& layout, Position agent,
              Position object) {
    const ParseResult result = parseWorld(layout);
    WorldState state{};
    if (result.world.has_value()) {
        state = result.world.value();
    }
    state.agent = agent;
    state.object = object;
    return state;
}

}  // namespace aih
