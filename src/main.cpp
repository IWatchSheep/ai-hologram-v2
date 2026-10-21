// Purpose: The program entry point. It prints the two
// header lines of plan Section 3.8 and then runs the
// scripted demonstration.
// Introduced in: Stage 0a (rebuild)
// Architecture: The first header line is built from
// projectName() and currentStage() in the core library.
// The demonstration itself lives in demo/scripted_demo.cpp
// and writes to std::cout, the standard output stream.

#include <iostream>

#include "core/project_info.h"
#include "demo/scripted_demo.h"

int main() {
    std::cout << aih::projectName() << " v2 -- Stage " << aih::currentStage()
              << "\n";
    std::cout << "Grid world: environment rules only (scripted "
                 "demonstration). No reasoning components yet.\n";
    aih::runScriptedDemo(std::cout);
    return 0;
}
