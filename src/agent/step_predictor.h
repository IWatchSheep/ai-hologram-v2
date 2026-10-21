// Purpose: Declares the per-step predictor: the single route
// through which every model evaluation of a reasoner is charged
// and cached.
// Introduced in: Stage 3c (rebuild)
// Architecture: StepPredictor owns one WorldModel and one
// BudgetCounter that is recreated at the start of every decision
// step (plan 3.4). The planner and all later checks obtain
// successor states only through it, so no successor state is ever
// created "for free" (protocol 10.7). It is the only place that
// calls WorldModel::predict for a reasoner. An optional observer
// (IE-1 H3) only watches: it reports charged evaluations and is
// never read back (rule R6).

#ifndef AI_HOLOGRAM_SRC_AGENT_STEP_PREDICTOR_H
#define AI_HOLOGRAM_SRC_AGENT_STEP_PREDICTOR_H

#include <array>
#include <optional>
#include <vector>

#include "agent/computation_observer.h"
#include "model/budget_counter.h"
#include "model/prediction.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/position.h"
#include "world/world_state.h"

namespace aih {

// All model evaluations of a reasoner go through this one class.
//
// The cache: within one decision step, repeating exactly the same
// prediction (same agent position, object position and action) is
// free. The cache also remembers a context: the walls and the
// destination the entries were computed under. That context is
// part of the cache in effect, because the same positions and
// action can give a different result when a wall differs: a wall
// can block the agent or the pushed object, and the destination
// belongs to the state the predictions were made from. So a
// prediction asked under other walls or another destination
// first clears the cache instead of reusing a result computed
// under the old context.
//
// A variant prediction is a prediction with a different model
// (such as the standard rule instead of the agent's current
// model), or from a hypothetical state. Variants are never read
// from the cache and never stored in it, because they do not
// describe what the reasoner's own model predicts from its own
// belief; each one always costs exactly one evaluation from the
// same step budget.
class StepPredictor {
public:
    // The model is the reasoner's own WorldModel; budgetPerStep
    // is B, the maximum evaluations charged in one decision step.
    StepPredictor(WorldModel model, int budgetPerStep);

    // Starts a new decision step: a fresh BudgetCounter with
    // limit B (unused budget from the last step is lost) and an
    // empty cache.
    void startStep();

    // Predicts action from state with the reasoner's own model.
    // A cache hit is free; a miss costs exactly one evaluation
    // and is stored in the cache. When the budget cannot pay
    // for a miss, this returns no prediction and changes
    // nothing: nothing is charged and nothing is cached.
    std::optional<Prediction> predict(const WorldState& state, Action action);

    // True when predict(state, action) would be served from the
    // cache, including the walls and destination context the
    // cache belongs to.
    bool isCached(const WorldState& state, Action action) const;

    // Predicts with model instead of the reasoner's own model.
    // Never reads or fills the cache, and costs exactly one
    // evaluation from the same step budget whenever the budget
    // can pay for it. It pays through WorldModel(model).predict.
    std::optional<Prediction> predictVariant(const WorldState& state,
                                             Action action,
                                             const ActionModel& model);

    // Attaches the watch-only observer for IE-1 H3 reports.
    void setObserver(const ComputationObserver* observer);

    // Empties the cache and removes its context, for when the
    // model or the believed walls change so that cached results
    // may no longer match what would be predicted now.
    void clearCache();

    // Evaluations still available in this step.
    int remaining() const;
    // Evaluations charged in this step (the counter's used).
    int usedThisStep() const;
    // Evaluations charged over all steps so far.
    int usedTotal() const;

private:
    // One cached prediction: the agent and object positions and
    // the action it was computed for, plus the prediction.
    struct CacheEntry {
        Position agent{};
        Position object{};
        Action action = Action::North;
        Prediction prediction{};
    };

    // Finds a cached entry for this key, or nullptr. The caller
    // must already have made sure the cache context belongs to
    // state's walls and destination.
    const CacheEntry* findCached(const WorldState& state, Action action) const;

    // Clears the cache when its context differs from state's
    // walls or destination (see predict).
    void clearCacheWhenContextDiffers(const WorldState& state);

    WorldModel model_;
    int budgetPerStep_;
    BudgetCounter budget_;
    std::vector<CacheEntry> cache_;

    // The walls and destination the cached entries were computed
    // under, or no context when hasCacheContext_ is false.
    bool hasCacheContext_ = false;
    std::array<std::array<bool, gridSize>, gridSize> cacheWalls_{};
    Position cacheDestination_{0, 0};

    int usedTotal_ = 0;
    const ComputationObserver* observer_ = nullptr;
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_STEP_PREDICTOR_H
