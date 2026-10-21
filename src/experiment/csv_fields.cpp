// Purpose: Implements the small CSV field helpers shared
// by the run row and the event row (rule R4).
// Introduced in: Stage 4f (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. One shared helper per job, so the two
// CSV files can never drift apart in how a field is
// spelled.

#include "experiment/csv_fields.h"

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace aih {

std::string joinFields(const std::vector<std::string>& fields) {
    std::string row;
    for (std::size_t i = 0; i < fields.size(); i = i + 1) {
        if (i > 0) {
            row += ",";
        }
        row += fields[i];
    }
    return row;
}

std::string bitText(bool value) {
    if (value) {
        return "1";
    }
    return "0";
}

std::string optionalStepText(const std::optional<int>& step) {
    if (!step.has_value()) {
        return "";
    }
    return std::to_string(step.value());
}

}  // namespace aih
