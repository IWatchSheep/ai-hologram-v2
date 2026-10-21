// Purpose: Implements alternativePrediction (plan 3.8):
// what an action would predict if a correction were undone.
// Introduced in: Stage 3d (rebuild)
// Architecture: This is the planned split from
// verification.cpp (plan Section 4). Every alternative is a
// variant prediction: for a model correction it uses the
// old model, and for a wall or object correction it uses
// the current model on the belief with the old value put
// back, so no alternative is ever read from or stored in
// the predictor's cache.

#include "agent/verification.h"

#include <cstddef>
#include <optional>

#include "agent/records.h"
#include "agent/step_predictor.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

std::optional<Prediction> alternativePrediction(
    const CorrectionRecord& correction, const WorldState& belief, Action action,
    StepPredictor& predictor, const ActionModel& currentModel,
    const ActionModel& oldModel) {
    const Position c1 = moved(belief.agent, action);
    const Position c2 = moved(c1, action);
    const Element& element = correction.element;
    if (element.kind == ElementKind::ActionModel) {
        if (element.action != action) {
            return std::nullopt;
        }
        if (!isValidWorld(belief)) {
            return std::nullopt;
        }
        return predictor.predictVariant(belief, action, oldModel);
    }
    if (element.kind == ElementKind::Trap) {
        return std::nullopt;
    }
    WorldState alternative = belief;
    if (element.kind == ElementKind::Wall) {
        if (element.cell != c1 && element.cell != c2) {
            return std::nullopt;
        }
        if (!isInsideGrid(element.cell)) {
            return std::nullopt;
        }
        const std::size_t x = static_cast<std::size_t>(element.cell.x);
        const std::size_t y = static_cast<std::size_t>(element.cell.y);
        alternative.walls[y][x] = correction.oldBlocked;
    } else {
        if (correction.oldPosition != c1 && correction.oldPosition != c2) {
            if (belief.object != c1 && belief.object != c2) {
                return std::nullopt;
            }
        }
        alternative.object = correction.oldPosition;
    }
    if (!isValidWorld(alternative)) {
        return std::nullopt;
    }
    return predictor.predictVariant(alternative, action, currentModel);
}

}  // namespace aih
