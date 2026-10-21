// Purpose: Tests for BudgetCounter (BC1 to BC4b, plan
// Section 6.1).
// Introduced in: Stage 2a (rebuild)
// Architecture: Each check builds its own counter, so the
// checks cannot affect each other. Expected values come
// from checks.py, the reference mirror in
// tools/reference/stage2/.

#include "model/budget_counter.h"
#include "test_suites.h"
#include "test_support.h"

namespace aih {
namespace {

void checkNewCounter(TestCounter& counter) {
    BudgetCounter budget(3);
    check(counter, budget.used() == 0 && budget.remaining() == 3,
          "BC1: BudgetCounter(3) starts with used 0, remaining 3");
}

void checkTryUseAndReset(TestCounter& counter) {
    BudgetCounter budget(3);
    const bool first = budget.tryUse();
    const bool second = budget.tryUse();
    const bool third = budget.tryUse();
    const bool fourth = budget.tryUse();
    check(counter, first && second && third && !fourth && budget.used() == 3,
          "BC2: tryUse four times gives true, true, true, false; used 3");
    budget.reset();
    check(counter, budget.used() == 0 && budget.tryUse(),
          "BC3: after reset(), used is 0 and tryUse is true");
}

void checkZeroAndNegative(TestCounter& counter) {
    BudgetCounter zero(0);
    check(counter, !zero.tryUse() && zero.used() == 0 && zero.remaining() == 0,
          "BC4a: BudgetCounter(0) refuses tryUse; used 0; remaining 0");
    BudgetCounter negative(-2);
    check(
        counter,
        !negative.tryUse() && negative.used() == 0 && negative.remaining() == 0,
        "BC4b: BudgetCounter(-2) behaves like 0; used 0; remaining 0");
}

}  // namespace

void runBudgetCounterTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Budget counter ---\n";
    }
    checkNewCounter(counter);
    checkTryUseAndReset(counter);
    checkZeroAndNegative(counter);
}

}  // namespace aih
