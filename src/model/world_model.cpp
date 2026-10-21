// Purpose: Implements budgeted predictions from a belief,
// exactly as plan Section 3.3 describes.
// Introduced in: Stage 2a (rebuild)
// Architecture: predict() pays through the budget counter
// first, then applies the stored action model to a copy of
// the belief's state inside the result, so the belief that
// was passed in is never changed.

#include "model/world_model.h"

#include <optional>

#include "model/belief.h"
#include "model/budget_counter.h"
#include "model/prediction.h"
#include "world/action.h"
#include "world/rules.h"

namespace aih {

WorldModel::WorldModel() : actionModel_(applyAction) {
}

WorldModel::WorldModel(ActionModel actionModel) : actionModel_(actionModel) {
    // Calling an empty std::function would throw, and this
    // project uses no exceptions, so fall back to the true
    // rules instead of ever calling an empty function.
    if (!actionModel_) {
        actionModel_ = applyAction;
    }
}

std::optional<Prediction> WorldModel::predict(const Belief& belief,
                                              Action action,
                                              BudgetCounter& budget) const {
    // Pay first: a refused payment means no prediction at
    // all, and the action model is not run.
    if (!budget.tryUse()) {
        return std::nullopt;
    }
    const ActionResult result = actionModel_(belief.state, action);
    return Prediction{action, result.outcome, result.state};
}

}  // namespace aih
