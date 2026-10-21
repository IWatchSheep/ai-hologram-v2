// Purpose: Declares the exact text of one traced run: the
// state, observation, mismatch, correction and result lines
// of the equivalence gate (plan 3.14).
// Introduced in: Stage 3g (rebuild)
// Architecture: ref_trace.py writes the golden traces in
// exactly this text, so the rebuilt trace writer must write
// the same bytes. Positions go through positionText and
// wall values through blockedText (rule R4).

#ifndef AI_HOLOGRAM_TESTS_TRACE_TEXT_H
#define AI_HOLOGRAM_TESTS_TRACE_TEXT_H

#include <string>
#include <vector>

#include "agent/observation.h"
#include "agent/records.h"
#include "world/world_state.h"

namespace aih {

// The 100 walls as 25 lowercase hex digits: cell 4j+k is
// bit k of digit j, the first cell its highest bit.
std::string hexWalls(const WorldState& state);

// "agent (x, y) object (x, y) destination (x, y) walls <hex>".
std::string stateText(const WorldState& state);

// "wall (x, y)", "object", "model <Action>" or "trap (x, y)".
std::string elementText(const Element& element);

// One delivered observation, with its last action and
// outcome, or "last none" when there was no action.
std::string observationLine(const Observation& observation);

// One mismatch record in full: its summary line, one line
// per differing footprint cell, one per candidate element.
std::vector<std::string> mismatchLines(const MismatchRecord& mismatch);

// One correction record: id, element, its new and old
// values for wall and object corrections, status, the step
// it became consistent at, and whether it was V2-tested.
std::string correctionLine(const CorrectionRecord& record);

// The result line at the end of a traced run, with every
// count the gate compares (plan 3.14).
std::string traceResultLine(const std::string& outcome, int worldSteps,
                            int decisionSteps, int safeStops, int mismatches,
                            int corrections, int verified, int evaluations);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_TRACE_TEXT_H
