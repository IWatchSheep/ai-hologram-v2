// Purpose: Declares the fault cases of the equivalence
// gate, part 2, and the bounds-safe reader for them.
// Introduced in: Stage 4g (rebuild)
// Architecture: A fault case is a Stage 3 trace case plus,
// just before its "end", one run line and its fault
// events. The Stage 3 reader (readTraceCases, unchanged)
// reads the case without those lines; this reader then
// reads the run and event lines case by case, in the
// order and with the problem texts of plan 3.15. Every
// line is checked to exist before it is read, numbers
// have at most nine digits, and nothing ever throws.

#ifndef AI_HOLOGRAM_TESTS_FAULT_CASE_READER_H
#define AI_HOLOGRAM_TESTS_FAULT_CASE_READER_H

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "experiment/fault_event.h"
#include "trace_cases.h"

namespace aih {

// One fault case: its Stage 3 case, the layout and run
// seeds of its run line, and its fault events in order.
struct FaultCase {
    TraceCase base;
    std::uint64_t layoutSeed = 0;
    int runSeed = 0;
    std::vector<FaultEvent> events;
};

// readFaultCases result: on failure cases is empty and
// problem says exactly what was wrong, in the words of
// plan 3.15 ("The reader").
struct FaultCaseRead {
    bool ok = false;
    std::vector<FaultCase> cases;
    std::string problem;
};

// Reads all fault cases in order, bounds-safe, never
// throwing, exactly as ref_fault_reader.read_fault_cases.
FaultCaseRead readFaultCases(const std::vector<std::string>& lines);

// The fault event of one "event ..." line, or nothing
// when the line is malformed. Types, targets, directions,
// rotations and actions are accepted only from the shared
// names (fault_event's name functions and actionName, R4).
std::optional<FaultEvent> parseFaultEventLine(const std::string& text);

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_FAULT_CASE_READER_H
