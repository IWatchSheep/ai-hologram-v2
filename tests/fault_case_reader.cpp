// Purpose: Implements the bounds-safe fault-case reader,
// following ref_fault_reader.py exactly (plan 3.15).
// Introduced in: Stage 4g (rebuild)
// Architecture: The Stage 3 reader first reads the file
// without its run and event lines; its problem is the
// result. Then each case must hold one run line and only
// valid event lines, counts checked before any read.
#include "fault_case_reader.h"
#include "golden_files.h"
#include "trace_case_parts.h"
namespace aih {
namespace {
using Lines = std::vector<std::string>;
using Words = std::vector<std::string>;
std::string firstWord(const std::string& text) {
    return splitWords(text)[0];
}
// Finds the value whose shared name (R4) is word. The
// fault enums are declared in name order from zero.
template <typename Value>
bool fromSharedName(const std::string& word, int count,
                    std::string (*nameOf)(Value), Value& value) {
    for (int index = 0; index < count; index = index + 1) {
        const Value candidate = static_cast<Value>(index);
        if (nameOf(candidate) == word) {
            value = candidate;
            return true;
        }
    }
    return false;
}
// Reads "cell X Y" or "cell none" at at; next index or -1.
int readCell(FaultEvent& event, const Words& words, int at) {
    const int size = static_cast<int>(words.size());
    if (at >= size || words[at] != "cell" || at + 1 >= size) {
        return -1;
    }
    if (words[at + 1] == "none") {
        return at + 2;
    }
    if (at + 2 >= size) {
        return -1;
    }
    const std::optional<int> x = traceInteger(words[at + 1]);
    const std::optional<int> y = traceInteger(words[at + 2]);
    if (!x.has_value() || !y.has_value()) {
        return -1;
    }
    event.hasCell = true;
    event.cell = Position{x.value(), y.value()};
    return at + 3;
}
// Reads one event's details from index 9, exactly filling it.
bool readDetails(FaultEvent& event, const Words& words) {
    const int size = static_cast<int>(words.size());
    if (event.type == FaultType::F1 || event.type == FaultType::F4) {
        const int at = readCell(event, words, 9);
        if (event.type == FaultType::F4) {
            return at == size;
        }
        if (at < 0 || at + 1 != size) {
            return false;
        }
        return fromSharedName(words[at], 2, f1DirectionName, event.direction);
    }
    if (event.type == FaultType::F2) {
        bool ok = size == 13;
        for (int k = 0; ok && k < 4; k = k + 1) {
            ok = actionFromName(words[9 + k], event.order[k]);
        }
        return ok;
    }
    if (event.type == FaultType::F3) {
        return size == 11 && actionFromName(words[9], event.action) &&
               fromSharedName(words[10], 2, rotationName, event.rotation);
    }
    return size == 10 && words[9] == "-";
}
// One case's run and events, from first to its "end" line.
bool readRunAndEvents(const Lines& lines, int first, int end, int number,
                      const TraceCase& base, FaultCase& faultCase,
                      std::string& problem) {
    const std::string prefix = "case " + std::to_string(number) + ": ";
    int at = first;
    while (at < end && firstWord(lines[at]) != "run" &&
           firstWord(lines[at]) != "event") {
        at = at + 1;
    }
    if (at >= end) {
        problem = prefix + "missing run line";
        return false;
    }
    const Words words = splitWords(lines[at]);
    std::optional<int> layoutSeed;
    std::optional<int> runSeed;
    if (words.size() == 3 && words[0] == "run") {
        layoutSeed = traceInteger(words[1]);
        runSeed = traceInteger(words[2]);
    }
    if (!layoutSeed.has_value() || !runSeed.has_value()) {
        problem = prefix + "malformed run line";
        return false;
    }
    faultCase.base = base;
    faultCase.layoutSeed = static_cast<std::uint64_t>(layoutSeed.value());
    faultCase.runSeed = runSeed.value();
    for (int index = at + 1; index < end; index = index + 1) {
        const std::optional<FaultEvent> event =
            parseFaultEventLine(lines[index]);
        if (!event.has_value()) {
            problem = prefix + "malformed event line";
            return false;
        }
        faultCase.events.push_back(event.value());
    }
    return true;
}
bool readEachCase(const Lines& lines, const TraceCaseRead& bases,
                  FaultCaseRead& output) {
    const int size = static_cast<int>(lines.size());
    int index = 0;
    for (std::size_t number = 0; number < bases.cases.size();
         number = number + 1) {
        if (index >= size || firstWord(lines[index]) != "case") {
            output.problem = "line " + std::to_string(index + 1) +
                             ": expected a case header";
            output.cases.clear();
            return false;
        }
        int end = index;
        while (end < size && lines[end] != "end") {
            end = end + 1;
        }
        FaultCase faultCase;
        const int caseNumber = static_cast<int>(number) + 1;
        if (!readRunAndEvents(lines, index, end, caseNumber,
                              bases.cases[number], faultCase, output.problem)) {
            output.cases.clear();
            return false;
        }
        output.cases.push_back(faultCase);
        index = end + 1;
    }
    if (index < size) {
        output.problem =
            "line " + std::to_string(index + 1) + ": expected a case header";
        output.cases.clear();
        return false;
    }
    return true;
}
}  // namespace
std::optional<FaultEvent> parseFaultEventLine(const std::string& text) {
    const Words words = splitWords(text);
    if (words.size() < 10 || words[0] != "event" || words[1].empty()) {
        return std::nullopt;
    }
    FaultEvent event;
    event.id = words[1];
    bool ok = (words[8] == "0" || words[8] == "1");
    int numbers[4] = {0, 0, 0, 0};
    for (int k = 0; ok && k < 4; k = k + 1) {
        const std::optional<int> number = traceInteger(words[2 + k]);
        ok = number.has_value();
        if (number.has_value()) {
            numbers[k] = number.value();
        }
    }
    event.layoutSeed = static_cast<std::uint64_t>(numbers[0]);
    event.runSeed = numbers[1];
    event.index = numbers[2];
    event.step = numbers[3];
    ok = ok && fromSharedName(words[6], 5, faultTypeName, event.type);
    ok = ok && fromSharedName(words[7], 3, faultTargetName, event.target);
    event.commonMode = words[8] == "1";
    if (!ok || !readDetails(event, words)) {
        return std::nullopt;
    }
    return event;
}
FaultCaseRead readFaultCases(const Lines& lines) {
    FaultCaseRead output;
    Lines stripped;
    for (const std::string& line : lines) {
        if (firstWord(line) != "run" && firstWord(line) != "event") {
            stripped.push_back(line);
        }
    }
    const TraceCaseRead bases = readTraceCases(stripped);
    if (!bases.ok) {
        output.problem = bases.problem;
        return output;
    }
    if (!readEachCase(lines, bases, output)) {
        return output;
    }
    output.ok = true;
    return output;
}
}  // namespace aih
