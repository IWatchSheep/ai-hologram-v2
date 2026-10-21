// Purpose: Implements readTraceCases, the bounds-safe reader
// of the equivalence gate's case file (plan 3.14).
// Introduced in: Stage 3g (rebuild)
// Architecture: This follows ref_trace_reader.py exactly: the
// same order of stages and problem texts. A small reader walks
// one case stage by stage, using the word readers of
// trace_case_parts; existence and word counts are checked
// before any read, so malformed input is never read out of
// range, and nothing throws.

#include "trace_cases.h"

#include <string>
#include <vector>

#include "golden_files.h"
#include "trace_case_parts.h"

namespace aih {
namespace {
using Lines = std::vector<std::string>;
using Words = std::vector<std::string>;

// Walks one case stage by stage, in the reference's order.
// Each stage either advances past its line and returns true,
// or sets the exact problem and returns false.
struct CaseReader {
    const Lines& lines;
    int number = 1;
    int index = 0;
    TraceCase result{};
    std::string problem{};
    std::string prefix() const {
        return "case " + std::to_string(number) + ": ";
    }
    // Sets the problem if none is set yet (a row parse has
    // already set its own), and always returns false.
    bool fail(const std::string& text) {
        if (problem.empty()) {
            problem = prefix() + text;
        }
        return false;
    }
    bool hasLine() const {
        return index < static_cast<int>(lines.size());
    }
    Words words() const {
        return hasLine() ? splitWords(lines[index]) : Words{""};
    }
    bool readRows(int first, const std::string& what, WorldState& world) {
        return traceWorldRows(lines, first, number, what, world, problem);
    }
    bool readHeader() {
        const Words head = words();
        const bool ok = head.size() == 2 && head[0] == "case";
        if (ok) {
            result.name = head[1];
            index = index + 1;
        }
        if (!ok) {
            problem = "line " + std::to_string(index + 1) +
                      ": expected a case header";
        }
        return ok;
    }
    bool readAgent() {
        const Words line = words();
        const bool ok = hasLine() && (line == Words{"agent", "C1"} ||
                                      line == Words{"agent", "C0"});
        if (ok) {
            result.agent = line[1];
            index = index + 1;
        }
        return ok ||
               fail(hasLine() ? "malformed agent line" : "missing agent line");
    }
    bool readSettings() {
        const bool ok =
            hasLine() && traceSettingsFromWords(words(), result.settings);
        if (ok) {
            index = index + 1;
        }
        return ok || fail(hasLine() ? "malformed settings line"
                                    : "missing settings line");
    }
    bool readWorld() {
        const bool ok = hasLine() && lines[index] == "world" &&
                        readRows(index + 1, "world", result.world);
        if (ok) {
            index = index + 11;
        }
        return ok || fail("missing world line");
    }
    bool readBelief() {
        const bool isTrue = hasLine() && lines[index] == "belief true";
        const bool isGiven = hasLine() && lines[index] == "belief given";
        WorldState belief;
        const bool rowsOk = !isGiven || readRows(index + 1, "belief", belief);
        const bool ok = (isTrue || isGiven) && rowsOk;
        if (ok && isGiven) {
            result.belief = belief;
            index = index + 10;
        }
        if (ok) {
            index = index + 1;
        }
        return ok || fail("malformed belief line");
    }
    bool readModel() {
        const Words line = words();
        const bool ok = hasLine() && traceModelWordsOk(line);
        if (ok) {
            result.modelSpec.assign(line.begin() + 1, line.end());
            index = index + 1;
        }
        return ok || fail("malformed model line");
    }
    bool readNotDead() {
        const bool ok =
            hasLine() && traceNotDeadFromWords(words(), result.notDead);
        if (ok) {
            index = index + 1;
        }
        return ok || fail("malformed notdead line");
    }
    bool readSeedsAndEnd() {
        while (hasLine() && words()[0] == "seed") {
            if (!applyTraceSeed(result, words())) {
                return fail("malformed seed line");
            }
            index = index + 1;
        }
        const bool endOk = hasLine() && lines[index] == "end";
        if (endOk) {
            index = index + 1;
        }
        return endOk || fail("missing end line");
    }
    bool readCase() {
        return readHeader() && readAgent() && readSettings() && readWorld() &&
               readBelief() && readModel() && readNotDead() &&
               readSeedsAndEnd();
    }
};
}  // namespace

TraceCaseRead readTraceCases(const Lines& lines) {
    TraceCaseRead output;
    CaseReader reader{lines};
    while (reader.hasLine()) {
        reader.number = static_cast<int>(output.cases.size()) + 1;
        reader.result = TraceCase{};
        reader.problem.clear();
        if (!reader.readCase()) {
            output.problem = reader.problem;
            return output;
        }
        output.cases.push_back(reader.result);
    }
    output.ok = true;
    return output;
}

}  // namespace aih
