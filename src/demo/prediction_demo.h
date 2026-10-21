// Purpose: Declares the prediction demonstrations: one with a
// correct belief and one with a wrong belief.
// Introduced in: Stage 2c (rebuild)
// Architecture: The demos use the shared demo layout, a
// WorldModel and a BudgetCounter, and write to a
// caller-provided stream, so the program and the test can
// both run them and receive the output where they need it.

#ifndef AI_HOLOGRAM_SRC_DEMO_PREDICTION_DEMO_H
#define AI_HOLOGRAM_SRC_DEMO_PREDICTION_DEMO_H

#include <iosfwd>

namespace aih {

// Runs both prediction demonstrations, writing to out.
// std::ostream& is a reference to an output stream: the
// caller chooses where the output goes. The program passes
// std::cout so the demos print on the screen, while the test
// passes a std::ostringstream so it can capture the text and
// compare it exactly.
void runPredictionDemos(std::ostream& out);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_DEMO_PREDICTION_DEMO_H
