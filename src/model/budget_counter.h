// Purpose: Declares the counter that pays for model
// evaluations, so predictions have a limited budget.
// Introduced in: Stage 2a (rebuild)
// Architecture: WorldModel must call tryUse() before every
// prediction. This counter is the only way a prediction
// may be paid for, so every condition gets the same budget.

#ifndef AI_HOLOGRAM_SRC_MODEL_BUDGET_COUNTER_H
#define AI_HOLOGRAM_SRC_MODEL_BUDGET_COUNTER_H

namespace aih {

// A class groups data with the operations that control it.
// Its members are private by default, so the counts below
// can only change through the functions declared here.
class BudgetCounter {
public:
    // A negative limit counts as 0, so it allows no use at
    // all. explicit prevents an int being turned into a
    // BudgetCounter by accident.
    explicit BudgetCounter(int limit);

    // If fewer than the limit have been used, adds 1 and
    // returns true. Otherwise changes nothing and returns
    // false. This is the only way to pay for a prediction.
    bool tryUse();

    // const member functions read the counts without
    // changing this counter.
    int used() const;
    int remaining() const;

    // Sets the used count back to 0.
    void reset();

private:
    int limit_;
    int used_;
};

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_MODEL_BUDGET_COUNTER_H
