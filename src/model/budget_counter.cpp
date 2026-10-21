// Purpose: Implements the model-evaluation budget counter
// of plan Section 3.2.
// Introduced in: Stage 2a (rebuild)
// Architecture: WorldModel consults this counter before
// every prediction, and no other code changes the counts.

#include "model/budget_counter.h"

namespace aih {

BudgetCounter::BudgetCounter(int limit)
    // The member initializer list sets the private counts
    // before the constructor body runs. A negative limit
    // is stored as 0, so remaining() can never go below 0
    // merely because of how the counter was built.
    : limit_(limit < 0 ? 0 : limit), used_(0) {
}

bool BudgetCounter::tryUse() {
    if (used_ >= limit_) {
        return false;
    }
    used_ = used_ + 1;
    return true;
}

int BudgetCounter::used() const {
    return used_;
}

int BudgetCounter::remaining() const {
    return limit_ - used_;
}

void BudgetCounter::reset() {
    used_ = 0;
}

}  // namespace aih
