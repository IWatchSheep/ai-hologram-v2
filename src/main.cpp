// Purpose: The program entry point. It prints the two
// header lines of plan Section 3.8 and then runs the
// scripted demonstration and the prediction demonstrations.
// Introduced in: Stage 0a (rebuild)
// Architecture: The first header line is built from
// projectName() and currentStage() in the core library.
// The demonstrations themselves live in src/demo/ and write
// to std::cout, the standard output stream.

#include <iostream>

#include "core/project_info.h"
#include "demo/prediction_demo.h"
#include "demo/scripted_demo.h"

int main() {
    std::cout << aih::projectName() << " v2 -- Stage " << aih::currentStage()
              << "\n";
    std::cout << "Grid world with prediction and comparison (scripted "
                 "demonstrations). No reasoning components yet.\n";
    aih::runScriptedDemo(std::cout);
    aih::runPredictionDemos(std::cout);
    return 0;
}
