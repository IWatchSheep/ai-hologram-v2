// Purpose: Implements the trap V1 and V2 tests, the
// prediction-difference test and the active-test targets
// (plan 3.8).
// Introduced in: Stage 3d (rebuild)
// Architecture: The trap V2 test predicts hypothetical
// states: the belief with the agent moved to each standing
// cell. A variant prediction is used because those states
// are not the agent's real position, so their predictions
// must not be cached as the agent's own.

#include "agent/verification.h"

#include <optional>
#include <utility>
#include <vector>

#include "agent/dead_squares.h"
#include "agent/directions.h"
#include "agent/history_checks.h"
#include "agent/observation.h"
#include "agent/step_predictor.h"
#include "model/prediction.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {
namespace {

// One hypothetical push of the trap V2 test: the state the
// agent would be in, and the push direction from there.
struct HypotheticalPush {
    WorldState state{};
    Action direction = Action::North;
};

}  // namespace

bool trapV1Check(const WorldState& belief, const DeadSquareMap& map, Position p,
                 const std::vector<Observation>& history,
                 FreeComputationCounts& counts) {
    if (localCheckSaysNotDead(belief, map, p, counts)) {
        return false;
    }
    return trapHistoryPasses(p, belief.destination, history);
}

TrapV2Result trapV2Test(const WorldState& belief, const DeadSquareMap& map,
                        Position p, StepPredictor& predictor,
                        const ActionModel& currentModel, int reserve,
                        int& anomalies) {
    std::vector<HypotheticalPush> pushes;
    for (const Action direction : kActionOrder) {
        const Position offset = offsetFor(direction);
        const Position stand{p.x - offset.x, p.y - offset.y};
        if (isWall(belief, stand)) {
            continue;
        }
        WorldState hypothetical = belief;
        hypothetical.agent = stand;
        hypothetical.object = p;
        if (!isValidWorld(hypothetical)) {
            anomalies = anomalies + 1;
            continue;
        }
        pushes.push_back(HypotheticalPush{hypothetical, direction});
    }
    const int kept = static_cast<int>(pushes.size());
    if (predictor.remaining() - kept < reserve) {
        return TrapV2Result::Waiting;
    }
    bool rejected = false;
    for (const HypotheticalPush& push : pushes) {
        const std::optional<Prediction> predicted =
            predictor.predictVariant(push.state, push.direction, currentModel);
        if (!predicted.has_value()) {
            return TrapV2Result::Waiting;
        }
        const Position movedTo = predicted->state.object;
        if (movedTo != p && isInsideGrid(movedTo)) {
            if (!readDeadStatus(map, movedTo)) {
                rejected = true;
            }
        }
    }
    if (rejected) {
        return TrapV2Result::Rejected;
    }
    return TrapV2Result::Verified;
}

bool predictionsDiffer(const Prediction& first, const Prediction& second) {
    if (first.outcome != second.outcome) {
        return true;
    }
    if (first.state.agent != second.state.agent) {
        return true;
    }
    return first.state.object != second.state.object;
}

std::vector<std::pair<Position, Action>> activeTestTargets(
    const WorldState& belief, Position cell) {
    std::vector<std::pair<Position, Action>> targets;
    for (const Action direction : kActionOrder) {
        const Position offset = offsetFor(direction);
        const Position stand{cell.x - offset.x, cell.y - offset.y};
        if (isWall(belief, stand)) {
            continue;
        }
        if (stand == belief.object) {
            continue;
        }
        targets.push_back(std::make_pair(stand, direction));
    }
    return targets;
}

}  // namespace aih
