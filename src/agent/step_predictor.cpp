// Purpose: Implements the per-step budget, prediction cache
// and evaluation counts of plan 3.4.
// Introduced in: Stage 3c (rebuild)
// Architecture: Every successor state a reasoner examines comes
// from a budgeted model evaluation made here (protocol 10.7).
// This class is the only place that calls WorldModel::predict
// for a reasoner, so the number of model calls always equals
// the charged evaluations. Charged evaluations are reported to
// the watch-only observer (IE-1 H3); the report changes nothing.

#include "agent/step_predictor.h"

#include <optional>
#include <utility>

#include "agent/computation_observer.h"
#include "model/belief.h"
#include "model/budget_counter.h"
#include "model/prediction.h"
#include "model/world_model.h"
#include "world/action.h"
#include "world/world_state.h"

namespace aih {

StepPredictor::StepPredictor(WorldModel model, int budgetPerStep)
    : model_(std::move(model)),
      budgetPerStep_(budgetPerStep),
      budget_(budgetPerStep) {
}

void StepPredictor::startStep() {
    // A brand-new counter: last step's unused budget is lost.
    budget_ = BudgetCounter(budgetPerStep_);
    clearCache();
}

void StepPredictor::setObserver(const ComputationObserver* observer) {
    observer_ = observer;
}

void StepPredictor::clearCache() {
    cache_.clear();
    hasCacheContext_ = false;
}

const StepPredictor::CacheEntry* StepPredictor::findCached(
    const WorldState& state, Action action) const {
    for (const CacheEntry& entry : cache_) {
        if (entry.agent == state.agent) {
            if (entry.object == state.object) {
                if (entry.action == action) {
                    return &entry;
                }
            }
        }
    }
    return nullptr;
}

bool StepPredictor::isCached(const WorldState& state, Action action) const {
    // Entries computed under other walls or another destination
    // do not count as cached for this state: they will be
    // cleared on predict, so they must not be trusted here.
    if (!hasCacheContext_) {
        return false;
    }
    if (state.walls != cacheWalls_) {
        return false;
    }
    if (state.destination != cacheDestination_) {
        return false;
    }
    return findCached(state, action) != nullptr;
}

void StepPredictor::clearCacheWhenContextDiffers(const WorldState& state) {
    // The cache belongs to one set of walls and one destination.
    // Other walls can change every result, so they clear the
    // cache first instead of reusing a stale prediction.
    if (hasCacheContext_) {
        if (state.walls != cacheWalls_) {
            clearCache();
        } else {
            if (state.destination != cacheDestination_) {
                clearCache();
            }
        }
    }
}

std::optional<Prediction> StepPredictor::predict(const WorldState& state,
                                                 Action action) {
    clearCacheWhenContextDiffers(state);
    const CacheEntry* entry = findCached(state, action);
    if (entry != nullptr) {
        // A cache hit is free (protocol 10.7): no budget is
        // spent and no observer report is made.
        return entry->prediction;
    }
    // A miss costs exactly one evaluation. WorldModel returns
    // nothing and charges nothing when the budget is exhausted;
    // in that case this predictor likewise changes nothing.
    const std::optional<Prediction> result =
        model_.predict(Belief{state}, action, budget_);
    if (!result.has_value()) {
        return std::nullopt;
    }
    usedTotal_ = usedTotal_ + 1;
    if (!hasCacheContext_) {
        cacheWalls_ = state.walls;
        cacheDestination_ = state.destination;
        hasCacheContext_ = true;
    }
    cache_.push_back(CacheEntry{state.agent, state.object, action, *result});
    if (observer_ != nullptr) {
        if (observer_->onEvaluation) {
            observer_->onEvaluation(state, action, nullptr, *result);
        }
    }
    return result;
}

std::optional<Prediction> StepPredictor::predictVariant(
    const WorldState& state, Action action, const ActionModel& model) {
    // A variant prediction uses a different model (or a
    // hypothetical state), so its result must never be reused
    // as if the reasoner's own model had made it: the cache is
    // neither read nor filled. It still pays exactly one
    // evaluation from the same step budget, through a
    // WorldModel built from the given model.
    const WorldModel variantModel(model);
    const std::optional<Prediction> result =
        variantModel.predict(Belief{state}, action, budget_);
    if (!result.has_value()) {
        return std::nullopt;
    }
    usedTotal_ = usedTotal_ + 1;
    if (observer_ != nullptr) {
        if (observer_->onEvaluation) {
            observer_->onEvaluation(state, action, &model, *result);
        }
    }
    return result;
}

int StepPredictor::remaining() const {
    return budget_.remaining();
}

int StepPredictor::usedThisStep() const {
    return budget_.used();
}

int StepPredictor::usedTotal() const {
    return usedTotal_;
}

}  // namespace aih
