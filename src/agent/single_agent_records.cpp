// Purpose: Finds and adds correction records, takes a
// safe stop, and applies and reverts corrections (plan 3.9).
// Introduced in: Stage 3e (rebuild)
// Architecture: Applying a wall correction only changes the
// believed wall: the step predictor clears its own cache at
// the next prediction, because the walls are part of its
// cache context (plan 3.4). Model and trap corrections
// change what is predicted or what counts as dead without
// changing the walls, so they clear the cache themselves.

#include "agent/single_agent.h"

#include <cstddef>
#include <string>

#include "agent/agent_log.h"
#include "agent/agent_types.h"
#include "agent/dead_squares.h"
#include "agent/records.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

CorrectionRecord* SingleAgent::findCorrection(int correctionId) {
    for (CorrectionRecord& record : state.corrections) {
        if (record.id == correctionId) {
            return &record;
        }
    }
    return nullptr;
}

CorrectionRecord& SingleAgent::addRecord(CorrectionRecord record) {
    record.id = state.nextCorrectionId;
    state.nextCorrectionId = state.nextCorrectionId + 1;
    state.corrections.push_back(record);
    return state.corrections.back();
}

Decision SingleAgent::safeStop(const std::string& reason) {
    // "Take no action" must always be possible: a safe stop
    // spends nothing, commits nothing and only logs its line.
    state.safeStops = state.safeStops + 1;
    state.committed.has = false;
    state.log.push_back(safeStopLine(state.decisionStep, reason));
    return Decision{false, Action::North, reason};
}

void SingleAgent::applyCorrection(const CorrectionRecord& record) {
    const Element& element = record.element;
    if (element.kind == ElementKind::Wall) {
        if (isInsideGrid(element.cell)) {
            const std::size_t x = static_cast<std::size_t>(element.cell.x);
            const std::size_t y = static_cast<std::size_t>(element.cell.y);
            state.belief.walls[y][x] = record.newBlocked;
        }
    }
    if (element.kind == ElementKind::Object) {
        state.belief.object = record.newPosition;
    }
    if (element.kind == ElementKind::ActionModel) {
        state.agentModel.restore(element.action);
        state.predictor.clearCache();
    }
    if (element.kind == ElementKind::Trap) {
        bool present = false;
        for (const Position& cell : state.deadOverrides) {
            if (cell == element.cell) {
                present = true;
            }
        }
        if (!present) {
            state.deadOverrides.push_back(element.cell);
        }
        state.predictor.clearCache();
    }
}

void SingleAgent::revertCorrection(const CorrectionRecord& record) {
    const Element& element = record.element;
    if (element.kind == ElementKind::Wall) {
        if (isInsideGrid(element.cell)) {
            const std::size_t x = static_cast<std::size_t>(element.cell.x);
            const std::size_t y = static_cast<std::size_t>(element.cell.y);
            state.belief.walls[y][x] = record.oldBlocked;
        }
    }
    if (element.kind == ElementKind::ActionModel) {
        state.agentModel.unrestore(element.action);
        state.predictor.clearCache();
    }
    if (element.kind == ElementKind::Trap) {
        std::vector<Position> kept;
        for (const Position& cell : state.deadOverrides) {
            if (cell != element.cell) {
                kept.push_back(cell);
            }
        }
        state.deadOverrides = kept;
        state.predictor.clearCache();
    }
    // An object correction is only marked when rejected:
    // there is nothing to put back (plan 3.9).
}

bool SingleAgent::objectDead(const WorldState& predicted,
                             const DeadSquareMap& map) const {
    // The one object-dead test (rule R4): inside the grid,
    // and dead through readDeadStatus (IE-1 H4).
    if (!isInsideGrid(predicted.object)) {
        return false;
    }
    return readDeadStatus(map, predicted.object);
}

}  // namespace aih
