// Purpose: Declares the small CSV field helpers shared by
// the run row and the event row (rule R4).
// Introduced in: Stage 4f (rebuild)
// Architecture: This is experimenter code in
// src/experiment/. CSV means comma-separated values: one
// line per record, fields separated by commas, and the
// first line naming the columns. Fields are joined with
// no quoting, so these helpers are the one copy of how a
// field is spelled.

#ifndef AI_HOLOGRAM_SRC_EXPERIMENT_CSV_FIELDS_H
#define AI_HOLOGRAM_SRC_EXPERIMENT_CSV_FIELDS_H

#include <optional>
#include <string>
#include <vector>

namespace aih {

// Joins fields with commas between them and no quoting.
std::string joinFields(const std::vector<std::string>& fields);

// The yes/no spelling of the CSV files: "1" or "0".
std::string bitText(bool value);

// A step number as text, or an empty field when the step
// does not exist. Empty means "did not happen"; it never
// means zero (D-044).
std::string optionalStepText(const std::optional<int>& step);

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_EXPERIMENT_CSV_FIELDS_H
