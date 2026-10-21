// Purpose: The program entry point. In Stage 0a it only prints
// two lines showing that the skeleton builds and runs.
// Introduced in: Stage 0a (rebuild)
// Architecture: The first line is built from projectName() and
// currentStage() in the core library, as the stage plan requires.

#include <iostream>

#include "core/project_info.h"

int main() {
    std::cout << aih::projectName() << " v2 -- Stage "
              << aih::currentStage() << "\n";
    std::cout << "Project skeleton: build, tests and code checks "
                 "only. No world, agents or experiments yet.\n";
    return 0;
}
