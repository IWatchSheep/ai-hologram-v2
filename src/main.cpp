// Purpose: The program entry point. It prints the three
// header lines of plan Section 5 and then runs the scripted
// demonstration, the prediction demonstrations, the
// agent demonstrations and the fault demonstration.
// Introduced in: Stage 0a (rebuild)
// Architecture: The first header line is built from
// projectName() and currentStage() in the core library.
// The demonstrations themselves live in src/demo/ and write
// to std::cout, the standard output stream.

#include <iostream>

#include "core/project_info.h"
#include "demo/agent_demo.h"
#include "demo/fault_demo.h"
#include "demo/prediction_demo.h"
#include "demo/scripted_demo.h"

int main() {
    std::cout << aih::projectName() << " v2 -- Stage " << aih::currentStage()
              << "\n";
    std::cout << "Grid world with a competent single agent (C1), a naive "
                 "agent (C0), fault injection and measurement.\n";
    std::cout << "Faults are injected only on demonstration material; no "
                 "two-agent reasoning exists, and no experiment is run.\n";
    aih::runScriptedDemo(std::cout);
    aih::runPredictionDemos(std::cout);
    aih::runAgentDemos(std::cout);
    aih::runFaultDemo(std::cout);
    return 0;
}
