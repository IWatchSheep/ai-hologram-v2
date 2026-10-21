// Purpose: Declares the records C1 keeps about mismatches
// and corrections, and the one reserve rule.
// Introduced in: Stage 3d (rebuild)
// Architecture: These are plain records with public members,
// so tests and (in 3e and 3g) the agent and the trace writer
// can read and set them (plan 3.9, D-019). reserveFor is the
// one copy of the reserve rule (rule R4).

#ifndef AI_HOLOGRAM_SRC_AGENT_RECORDS_H
#define AI_HOLOGRAM_SRC_AGENT_RECORDS_H

#include <vector>

#include "agent/footprint.h"
#include "world/action.h"
#include "world/position.h"

namespace aih {

// Where a suspicion or a correction came from (plan 3.7).
// The investigation uses S1 (the action model), S2 (a belief
// candidate checked against the record) and S5 (no candidate
// passed); S3 and S4 are named by the protocol for later use.
enum class SourceLabel { S1, S2, S3, S4, S5 };

// A correction is Consistent when it has been made and not
// yet tested, Verified when a later step agreed with it, and
// Rejected when a later step disagreed with it.
enum class CorrectionStatus { Consistent, Verified, Rejected };

// What one correction is about: a wall cell, the object
// position, the action model for one action, or a trap cell.
enum class ElementKind { Wall, Object, ActionModel, Trap };

// The part of the world one correction is about. cell is used
// by wall and trap corrections, action by model corrections.
struct Element {
    ElementKind kind = ElementKind::Wall;
    Position cell{};
    Action action = Action::North;
};

// One correction and its bookkeeping. The new and old values
// are kept for both wall corrections (blocked or free) and
// object corrections (positions), so a rejected correction
// can be reverted and a trace can show what changed.
struct CorrectionRecord {
    int id = 0;
    Element element{};
    bool newBlocked = false;
    bool oldBlocked = false;
    Position newPosition{};
    Position oldPosition{};
    CorrectionStatus status = CorrectionStatus::Consistent;
    int stepConsistent = 0;
    bool tested = false;
};

// One recorded mismatch: the step of the check that found it
// (plan 3.15 Q5), the action, the footprint differences, the
// candidate elements, the suspected source and the anomaly
// flag (the believed agent differed from the record).
struct MismatchRecord {
    int decisionStep = 0;
    Action action = Action::North;
    FootprintComparison differences{};
    std::vector<Element> candidates;
    SourceLabel suspected = SourceLabel::S5;
    bool anomaly = false;
};

// The reserve: 1 plus the number of Consistent corrections
// whose kind is not Trap. The reserve is the number of
// evaluations that must always be kept back for the step's
// mandatory predictions (plan 3.8), so optional work never
// spends the budget the committed prediction needs. This is
// the one copy of that rule (rule R4).
inline int reserveFor(const std::vector<CorrectionRecord>& corrections) {
    int reserve = 1;
    for (const CorrectionRecord& record : corrections) {
        if (record.status == CorrectionStatus::Consistent) {
            if (record.element.kind != ElementKind::Trap) {
                reserve = reserve + 1;
            }
        }
    }
    return reserve;
}

}  // namespace aih

#endif  // AI_HOLOGRAM_SRC_AGENT_RECORDS_H
