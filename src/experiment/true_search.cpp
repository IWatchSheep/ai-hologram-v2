// Purpose: Implements the experimenter's breadth-first
// search for the shortest solution of a true world.
// Introduced in: Stage 4a (rebuild)
// Architecture: The queue is a vector read from the front
// with an index, so states are visited in order of depth and
// the first solution found is a shortest one. Visited states
// are remembered by (agent, object) only, in one flat array.

#include "experiment/true_search.h"

#include <array>
#include <cstddef>
#include <optional>
#include <vector>

#include "agent/directions.h"
#include "world/action.h"
#include "world/position.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {
namespace {

// One queued state: the world and its depth in world steps.
struct SearchNode {
    WorldState state{};
    int depth = 0;
};

// A flat index for one (agent, object) pair, so the visited
// marks fit in one array of 10 * 10 * 10 * 10 entries.
int pairIndex(Position agent, Position object) {
    const int agentCell = agent.y * gridSize + agent.x;
    const int objectCell = object.y * gridSize + object.x;
    return agentCell * gridSize * gridSize + objectCell;
}

}  // namespace

std::optional<int> shortestSolutionLength(const WorldState& world) {
    if (world.object == world.destination) {
        return 0;
    }
    std::array<bool, gridSize * gridSize * gridSize * gridSize> seen{};
    std::vector<SearchNode> queue;
    queue.push_back(SearchNode{world, 0});
    seen[static_cast<std::size_t>(pairIndex(world.agent, world.object))] = true;
    for (std::size_t front = 0; front < queue.size(); front = front + 1) {
        const SearchNode node = queue[front];
        for (const Action action : kActionOrder) {
            const ActionResult next = applyAction(node.state, action);
            if (next.outcome == MoveOutcome::Blocked) {
                continue;
            }
            if (next.outcome == MoveOutcome::InvalidState) {
                continue;
            }
            const int index = pairIndex(next.state.agent, next.state.object);
            if (seen[static_cast<std::size_t>(index)]) {
                continue;
            }
            seen[static_cast<std::size_t>(index)] = true;
            if (next.state.object == next.state.destination) {
                return node.depth + 1;
            }
            queue.push_back(SearchNode{next.state, node.depth + 1});
        }
    }
    return std::nullopt;
}

}  // namespace aih
