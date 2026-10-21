// Purpose: Declares the agent's own action model: a base model
// plus the actions that have been restored to the standard rule.
// Introduced in: Stage 3c (rebuild)
// Architecture: A model(a) correction never rewrites the base
// model. It only marks action a as restored, and current()
// builds the ActionModel the StepPredictor should use from then
// on. Building that model evaluates nothing; evaluations happen
// only when the StepPredictor calls the model and charges the
// step budget. setBase is the IE-1 H2 hook: only the Stage 4
// fault injector will call it; agent code never does.

#ifndef AI_HOLOGRAM_SRC_AGENT_AGENT_MODEL_H
#define AI_HOLOGRAM_SRC_AGENT_AGENT_MODEL_H

#include <array>

#include "model/world_model.h"
#include "world/action.h"

namespace aih {

// The agent's action model (plan 3.5). In Experiment 1 the world
// rules are fully known, so the only candidate correction of
// model(a) is "use the standard rule (applyAction) for a". An
// empty model always means applyAction, so an empty function is
// never called.
class AgentActionModel {
public:
    // With no base model given, the base model is applyAction.
    AgentActionModel();

    // Uses the given base model, or applyAction if it is empty.
    explicit AgentActionModel(ActionModel baseModel);

    // Marks action as restored to the standard rule (applying a
    // model(action) correction), or removes that mark again
    // (reverting a correction that was later rejected).
    void restore(Action action);
    void unrestore(Action action);
    bool isRestored(Action action) const;

    // Replaces the base model and keeps the restored marks
    // (IE-1 H2). An empty model means applyAction. Only the
    // Stage 4 fault injector will call this; in Stage 3 only
    // test AM5 does.
    void setBase(ActionModel model);

    // The model to predict with now: applyAction for restored
    // actions, the base model for all others. It is a snapshot:
    // it copies the base model and the four marks when called,
    // so a model handed out before a restoration keeps the old
    // marks (plan 3.15 Q2). The StepPredictor's model asks for
    // current() at every evaluation, so the agent itself always
    // predicts with the up-to-date model.
    ActionModel current() const;

    // The base model in use before any restoration. This is the
    // "old model" of a model(a) correction.
    ActionModel base() const;

private:
    ActionModel baseModel_;
    // One mark per action, in kActionOrder (North, East, South,
    // West), so index 0 is North and index 3 is West.
    std::array<bool, 4> restored_{};
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_AGENT_MODEL_H
