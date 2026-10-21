// Purpose: The program entry point. It prints the three
// header lines of plan 3.12 and then runs the scripted
// demonstration, the prediction demonstrations and the
// agent demonstrations.
// Introduced in: Stage 0a (rebuild)
// Architecture: The first header line is built from
// projectName() and currentStage() in the core library.
// The demonstrations themselves live in src/demo/ and write
// to std::cout, the standard output stream.

#include <iostream>

#include "core/project_info.h"
#include "demo/agent_demo.h"
#include "demo/prediction_demo.h"
#include "demo/scripted_demo.h"

int main() {
    std::cout << aih::projectName() << " v2 -- Stage " << aih::currentStage()
              << "\n";
    std::cout << "Grid world with a competent single agent (C1), a naive "
                 "agent (C0) and scripted demonstrations.\n";
    std::cout << "No faults are injected, no two-agent reasoning exists, "
                 "and no experiment is run.\n";
    aih::runScriptedDemo(std::cout);
    aih::runPredictionDemos(std::cout);
    aih::runAgentDemos(std::cout);
    return 0;
}
