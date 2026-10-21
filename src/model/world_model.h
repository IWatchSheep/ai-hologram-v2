// Purpose: Declares the world model that predicts the
// result of an action from a belief, for a budget payment.
// Introduced in: Stage 2a (rebuild)
// Architecture: The model uses the true rules (applyAction)
// by default, or a supplied action model instead. Every
// prediction goes through BudgetCounter::tryUse() first.

#ifndef AI_HOLOGRAM_SRC_MODEL_WORLD_MODEL_H
#define AI_HOLOGRAM_SRC_MODEL_WORLD_MODEL_H

#include <functional>
#include <optional>

#include "model/belief.h"
#include "model/budget_counter.h"
#include "model/prediction.h"
#include "world/action.h"
#include "world/rules.h"
#include "world/world_state.h"

namespace aih {

// A "using" alias gives a long type a short name. Here it
// names the shape of an action model: a callable that takes
// a world state and an action, and returns an ActionResult.
// std::function can store any callable with that shape, such
// as applyAction or a function a test supplies. Using it
// lets a different model be plugged in without inheritance:
// no base class or derived classes are needed.
using ActionModel = std::function<ActionResult(const WorldState&, Action)>;

// Predicts from a belief. It never changes the belief, and
// it holds no true world of its own.
class WorldModel {
public:
    // Uses the true rules, applyAction.
    WorldModel();

    // Uses the given model instead. If the given function
    // is empty, the true rules are used instead, so an
    // empty function is never called (calling one throws).
    explicit WorldModel(ActionModel actionModel);

    // Pays with budget.tryUse() first. If payment is
    // refused, returns an empty std::optional, which here
    // means "no prediction". Otherwise applies the action
    // model to belief.state and returns the prediction.
    // This function is const: it does not change this
    // WorldModel. It can still change the caller's budget,
    // because budget is a non-const reference parameter:
    // const applies to this object, not to the arguments.
    std::optional<Prediction> predict(const Belief& belief, Action action,
                                      BudgetCounter& budget) const;

private:
    ActionModel actionModel_;
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_MODEL_WORLD_MODEL_H
