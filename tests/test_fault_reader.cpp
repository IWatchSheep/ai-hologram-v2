// Purpose: Tests for the fault-case reader (FN1 to FN10)
// and the fault trace text (FT1, FT2; plan 6.6).
// Introduced in: Stage 4g (rebuild)
// Architecture: Each malformed case must give exactly the
// problem text of checks.py section 4g, which makes the
// reader safe on a golden file that may be wrong: it
// reports the first problem instead of reading past the
// end. FN10 is the positive control; FT1, FT2 pin the text.

#include <string>
#include <vector>

#include "demo/scripted_demo.h"
#include "experiment/fault_event.h"
#include "experiment/fault_injector.h"
#include "experiment/scorer_types.h"
#include "fault_case_reader.h"
#include "fault_trace_text.h"
#include "stage4_suites.h"
#include "test_support.h"
#include "world/action.h"
#include "world/position.h"

namespace aih {
namespace {
using Lines = std::vector<std::string>;

// One fault case as case_lines of checks.py builds it:
// the demo layout, the middle (run) lines, the tail lines.
Lines faultCaseLines(const std::string& name, const Lines& middle,
                     const Lines& tail) {
    Lines lines{"case " + name, "agent C1", "settings 200 5 100 3", "world"};
    const Lines layout = demoLayout();
    lines.insert(lines.end(), layout.begin(), layout.end());
    lines.push_back("belief true");
    lines.push_back("model standard");
    lines.push_back("notdead 0");
    lines.insert(lines.end(), middle.begin(), middle.end());
    lines.insert(lines.end(), tail.begin(), tail.end());
    lines.push_back("end");
    return lines;
}

// The seven event lines of checks.py EVENTS_ALL.
Lines allEventLines() {
    return Lines{"event x-1 7 2 1 1 F1 R1 0 cell 4 3 blocked->free",
                 "event x-2 7 2 2 2 F1 both 1 cell none free->blocked",
                 "event x-3 7 2 3 3 F2 R2 0 West North East South",
                 "event x-4 7 2 4 4 F3 R1 0 South left",
                 "event x-5 7 2 5 5 F4 R2 0 cell 1 1",
                 "event x-6 7 2 6 6 F4 both 1 cell none",
                 "event x-7 7 2 7 7 F5 R1 0 -"};
}

// One reader check: exactly (not ok, no cases, problem).
void checkProblem(TestCounter& counter, const std::string& id,
                  const Lines& lines, const std::string& problem) {
    const FaultCaseRead result = readFaultCases(lines);
    const bool good =
        !result.ok && result.cases.empty() && result.problem == problem;
    std::string description = id + ": readFaultCases problem";
    if (!good) {
        description += " (actual problem [" + result.problem + "])";
    }
    check(counter, good, description);
}

// FN1 to FN9, with the exact texts of READER_CASES.
void checkReaderProblems(TestCounter& counter) {
    Lines noEnd = faultCaseLines("x", Lines{"run 7 2"}, Lines{});
    noEnd.pop_back();
    checkProblem(counter, "FN1", noEnd, "case 1: missing end line");
    checkProblem(counter, "FN2", faultCaseLines("x", Lines{}, Lines{}),
                 "case 1: missing run line");
    Lines badAgent = faultCaseLines("x", Lines{"run 7 2"}, Lines{});
    badAgent[1] = "agent C2";
    checkProblem(counter, "FN3", badAgent, "case 1: malformed agent line");
    checkProblem(counter, "FN4", faultCaseLines("x", Lines{"run 7"}, Lines{}),
                 "case 1: malformed run line");
    const Lines badEvents{"event x-1 7 2 1 1 F6 R1 0 -",
                          "event x-1 7 2 1 1 F1 R1 0 cell 4 blocked->free",
                          "event x-1 7 2 1 1 F2 R1 0 North East South",
                          "event x-1 7 2 1 1 F3 R1 0 North up"};
    for (int k = 0; k < 4; k = k + 1) {
        const std::string id = "FN" + std::to_string(5 + k);
        checkProblem(counter, id,
                     faultCaseLines("x", Lines{"run 7 2"}, Lines{badEvents[k]}),
                     "case 1: malformed event line");
    }
    Lines stray = faultCaseLines("x", Lines{"run 7 2"}, Lines{});
    stray.push_back("event x-1 7 2 1 1 F5 R1 0 -");
    checkProblem(counter, "FN9", stray, "line 20: expected a case header");
}

// True when the seven events are exactly EVENTS_ALL.
bool eventsAreExact(const std::vector<FaultEvent>& events) {
    bool ok = events.size() == 7;
    if (!ok) {
        return false;
    }
    const FaultType types[] = {FaultType::F1, FaultType::F1, FaultType::F2,
                               FaultType::F3, FaultType::F4, FaultType::F4,
                               FaultType::F5};
    for (std::size_t i = 0; i < events.size(); i = i + 1) {
        ok = ok && events[i].type == types[i];
    }
    ok = ok && events[0].hasCell && events[0].cell == Position{4, 3};
    ok = ok && events[0].direction == F1Direction::BlockedToFree;
    ok = ok && !events[1].hasCell && events[1].commonMode &&
         events[1].target == FaultTarget::Both;
    const Action order[] = {Action::West, Action::North, Action::East,
                            Action::South};
    for (int k = 0; k < 4; k = k + 1) {
        ok = ok && events[2].order[k] == order[k];
    }
    ok = ok && events[3].action == Action::South &&
         events[3].rotation == Rotation::Left;
    ok = ok && events[4].cell == Position{1, 1} && !events[5].hasCell;
    ok = ok && events[6].id == "x-7" && events[6].step == 7 &&
         events[6].index == 7 && events[6].layoutSeed == 7;
    return ok;
}

// FN10, the positive control: two cases, one with seven
// events covering every type, the second with none.
void checkPositiveControl(TestCounter& counter) {
    Lines lines = faultCaseLines("x", Lines{"run 7 2"}, allEventLines());
    const Lines second = faultCaseLines("y", Lines{"run 0 0"}, Lines{});
    lines.insert(lines.end(), second.begin(), second.end());
    const FaultCaseRead result = readFaultCases(lines);
    bool ok = result.ok && result.problem.empty() && result.cases.size() == 2;
    if (ok) {
        ok = result.cases[1].events.empty() && result.cases[1].base.name == "y";
        ok = ok && result.cases[0].layoutSeed == 7 &&
             result.cases[0].runSeed == 2;
        ok = ok && eventsAreExact(result.cases[0].events);
    }
    check(counter, ok, "FN10: two cases, every event field exact");
}

// FT1: the activation lines for the four statuses.
void checkActivationText(TestCounter& counter) {
    ActivationRecord record;
    record.eventId = "a-1";
    record.scheduledStep = 3;
    Lines lines;
    lines.push_back(faultActivationLine(record));
    record.status = ActivationStatus::Pending;
    record.reason = "the object has not moved yet";
    lines.push_back(faultActivationLine(record));
    record.status = ActivationStatus::Applied;
    record.applicationStep = 5;
    record.reason = "";
    lines.push_back(faultActivationLine(record));
    record.status = ActivationStatus::NotApplicable;
    record.reason = "no cell of the required kind";
    lines.push_back(faultActivationLine(record));
    const Lines expected{
        "activation a-1 scheduled",
        "activation a-1 pending | the object has not moved yet",
        "activation a-1 applied at 5",
        "activation a-1 not-applicable | no cell of the required kind"};
    check(counter, lines == expected, "FT1: the four activation lines exactly");
}

// FT2: one score line, exactly.
void checkScoreText(TestCounter& counter) {
    EventScore score;
    score.activated = true;
    score.activationStep = 2;
    score.endState = EndState::Corrected;
    score.endStep = 4;
    score.correctingId = 3;
    score.opportunityStep = 2;
    score.revealing = true;
    score.strictStep = 4;
    const std::string line = faultScoreLine("a-1", score);
    check(counter,
          line ==
              "score a-1 activated yes at 2 active no end corrected 4 by 3 "
              "opportunity 2 revealing yes mismatch - strict 4",
          "FT2: one score line exactly");
}
}  // namespace

void runFaultReaderTests(TestCounter& counter) {
    if (counter.verbose) {
        *counter.out << "--- Fault reader ---\n";
    }
    checkReaderProblems(counter);
    checkPositiveControl(counter);
    checkActivationText(counter);
    checkScoreText(counter);
}

}  // namespace aih
