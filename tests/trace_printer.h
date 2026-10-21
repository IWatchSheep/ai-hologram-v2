// Purpose: Declares the trace printer: it turns the run
// loop watchers' reports into trace or observer lines,
// and the tally the watch-only observer counts (plan 3.14).
// Introduced in: Stage 3g (rebuild)
// Architecture: This is the C1Printer of ref_trace.py. It
// only reads the agents' public state after a decision or
// the final check, and the observer only counts reports
// (rule R6: the camera cannot pull the chain), so a traced
// run is exactly the run itself. trace_writer builds and
// runs the agents; this printer writes what they did.

#ifndef AI_HOLOGRAM_TESTS_TRACE_PRINTER_H
#define AI_HOLOGRAM_TESTS_TRACE_PRINTER_H

#include <cstddef>
#include <string>
#include <vector>

#include "agent/agent_types.h"
#include "agent/computation_observer.h"
#include "agent/naive_agent.h"
#include "agent/observation.h"
#include "agent/single_agent.h"

namespace aih {

// Counts of the IE-1 reports since the last observer line.
struct Tally {
    int own = 0;
    int variant = 0;
    int deadReads = 0;
    int deadTrue = 0;
    int heuristics = 0;
    int infinite = 0;
    int sum = 0;
};

// The watch-only observer (H3, H4) that counts into a
// tally: charged own and variant evaluations, dead reads,
// and heuristic values. Nothing reads it back.
ComputationObserver makeObserver(Tally& tally);

// Writes one run's lines as the watchers report its steps.
struct Printer {
    std::vector<std::string> out;
    Tally tally;
    bool observers = false;
    std::size_t logShown = 0;
    std::size_t mismatchesShown = 0;
    std::vector<std::string> correctionsShown;

    // The step or final head, plus the observer line in
    // observer mode, counting since the last such line.
    void stepHead(const std::string& head);
    // The log, mismatch and correction lines since the
    // last traced step; a correction prints when new or
    // changed since it was last printed.
    void appendRecords(const SingleAgent& agent);
    // The lines every C1 step and the final check share.
    void appendCommon(const SingleAgent& agent);
    // One C1 decision step, after the agent has decided.
    void c1Step(const SingleAgent& agent, const Observation& given,
                const Decision& decision);
    // The C1 final check, after the run has ended.
    void c1Final(const SingleAgent& agent, const Observation& given);
    // One C0 decision step, after the agent has decided.
    void c0Step(const NaiveAgent& agent, const Observation& given,
                const Decision& decision);
};

}  // namespace aih

#endif  // AI_HOLOGRAM_TESTS_TRACE_PRINTER_H
