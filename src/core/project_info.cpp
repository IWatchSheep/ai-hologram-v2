// Purpose: Defines the project's name and current stage number.
// Introduced in: Stage 0a (rebuild)
// Architecture: The values live here only, so later stages change
// the stage number in exactly one place.

#include "core/project_info.h"

#include <string>

namespace aih {

std::string projectName() {
    return "AI Hologram";
}

int currentStage() {
    return 2;
}

}  // namespace aih
