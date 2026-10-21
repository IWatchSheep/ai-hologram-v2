// Purpose: Declares the fixed scripted demonstration for the
// grid world.
// Introduced in: Stage 1c (rebuild)
// Architecture: The demo uses the core world components and
// writes to a caller-provided stream, so the program and the
// test can both run it and receive its output where they need.

#ifndef AI_HOLOGRAM_SRC_DEMO_SCRIPTED_DEMO_H
#define AI_HOLOGRAM_SRC_DEMO_SCRIPTED_DEMO_H

#include <iosfwd>

namespace aih {

// Runs the scripted demonstration, writing to out.
// std::ostream& is a reference to an output stream: the
// caller chooses where the output goes. The program passes
// std::cout so the demo prints on the screen, while the test
// passes a std::ostringstream so it can capture the text and
// compare it exactly. Taking the stream as a parameter is
// what makes that possible without changing the demo.
void runScriptedDemo(std::ostream& out);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_DEMO_SCRIPTED_DEMO_H
