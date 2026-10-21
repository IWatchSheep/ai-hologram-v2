// Purpose: Declares the project's name and current stage number,
// so the program and the tests read them from one place.
// Introduced in: Stage 0a (rebuild)
// Architecture: Plain functions in namespace aih, declared here and
// defined in project_info.cpp. No state is stored anywhere.

#ifndef AI_HOLOGRAM_SRC_CORE_PROJECT_INFO_H
#define AI_HOLOGRAM_SRC_CORE_PROJECT_INFO_H

#include <string>

namespace aih {

// Returns the display name of the project.
std::string projectName();

// Returns the number of the stage this build belongs to.
int currentStage();

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_CORE_PROJECT_INFO_H
