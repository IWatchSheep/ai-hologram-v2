// Purpose: Defines the expected 68 lines of the Stage 4
// demonstration, copied exactly from lines 103-170 of
// tools/reference/stage4_rebuild/expected_program_output.txt.
// Introduced in: Stage 4f (rebuild)
// Architecture: Plain expected text for FD1, not results.

#include "expected_fault_demo.h"

#include <string>
#include <vector>

namespace aih {
namespace {

// The heading and the layout drawing.
void addOpeningLines(std::vector<std::string>& lines) {
    lines.push_back("");
    lines.push_back(
        "Stage 4: the measuring instrument, on demonstration "
        "material (not an experiment)");
    lines.push_back(
        "Faults are injected into a reasoner's beliefs or "
        "reasoning, never into the world.");
    lines.push_back(
        "Demonstration seeds 500000-500003 only; no pilot or "
        "confirmatory layout is used.");
    lines.push_back("");
    lines.push_back(
        "Layout 500000 (found at attempt 3, shortest solution 12 "
        "world steps):");
    lines.push_back("##########");
    lines.push_back("#...#....#");
    lines.push_back("#.....#A.#");
    lines.push_back("##.......#");
    lines.push_back("#.#......#");
    lines.push_back("#.....O..#");
    lines.push_back("##D...#..#");
    lines.push_back("#........#");
    lines.push_back("#......#.#");
    lines.push_back("##########");
}

// The six fault events of runs 500000-1 and 500003-1.
void addEventListLines(std::vector<std::string>& lines) {
    lines.push_back("");
    lines.push_back(
        "Fault events of runs 500000-1 and 500003-1 (fault master "
        "seed 4242):");
    lines.push_back("500000-1-1 step 4 F1 target both | (6, 8) free->blocked");
    lines.push_back("500000-1-2 step 6 F3 target R1 | East right");
    lines.push_back("500000-1-3 step 10 F4 target R2 | (8, 2)");
    lines.push_back("500003-1-1 step 1 F5 target R1 | -");
    lines.push_back("500003-1-2 step 2 F4 target R1 | (1, 7)");
    lines.push_back("500003-1-3 step 8 F1 target R2 | (6, 8) free->blocked");
}

// The two runs of 500000-2 and their activation lines.
void addRunLines(std::vector<std::string>& lines) {
    lines.push_back("");
    lines.push_back(
        "Run 500000-2, C1: Success after 12 world steps, 12 "
        "decision steps, 0 safe stops, 304 model evaluations");
    lines.push_back(
        "  500000-2-1 F1: Activated at its scheduled step (step 2)");
    lines.push_back(
        "  500000-2-2 F2: Activated at its scheduled step (step 9)");
    lines.push_back(
        "  500000-2-3 F4: Activated at its scheduled step (step 10)");
    lines.push_back("");
    lines.push_back(
        "Run 500000-2, C0: Success after 12 world steps, 12 "
        "decision steps, 0 safe stops, 152 model evaluations");
    lines.push_back(
        "  500000-2-1 F1: Activated at its scheduled step (step 2)");
    lines.push_back(
        "  500000-2-2 F2: Activated at its scheduled step (step 9)");
    lines.push_back(
        "  500000-2-3 F4: Activated at its scheduled step (step 10)");
}

// The two runs of 500003-1 and their activation lines.
void addSecondRunLines(std::vector<std::string>& lines) {
    lines.push_back("");
    lines.push_back(
        "Run 500003-1, C1: Success after 16 world steps, 16 "
        "decision steps, 0 safe stops, 525 model evaluations");
    lines.push_back(
        "  500003-1-1 F5: Activated after deferral (step 9, delay 8)");
    lines.push_back(
        "  500003-1-2 F4: Activated at its scheduled step (step 2)");
    lines.push_back(
        "  500003-1-3 F1: Activated at its scheduled step (step 8)");
    lines.push_back("");
    lines.push_back(
        "Run 500003-1, C0: Success after 16 world steps, 16 "
        "decision steps, 0 safe stops, 256 model evaluations");
    lines.push_back(
        "  500003-1-1 F5: Activated after deferral (step 11, delay 10)");
    lines.push_back(
        "  500003-1-2 F4: Activated at its scheduled step (step 2)");
    lines.push_back(
        "  500003-1-3 F1: Activated at its scheduled step (step 8)");
}

// The runs.csv section.
void addRunCsvLines(std::vector<std::string>& lines) {
    lines.push_back("");
    lines.push_back("runs.csv:");
    lines.push_back(
        "run_id,condition,layout_seed,run_seed,outcome,decision_steps,world_"
        "steps,safe_stops,justified_safe_stops,mismatches,mismatches_without_"
        "fault_cause,source_labels_correct,source_labels_scored,claims,false_"
        "alarms,wrong_accepted_corrections,evaluations,events_scheduled,events_"
        "activated");
    lines.push_back(
        "500000-2,C1,500000,2,Success,12,12,0,0,1,0,1,1,1,0,0,304,3,3");
    lines.push_back(
        "500000-2,C0,500000,2,Success,12,12,0,0,0,0,0,0,0,0,0,152,3,3");
    lines.push_back(
        "500003-1,C1,500003,1,Success,16,16,0,0,1,0,1,1,1,0,0,525,3,3");
    lines.push_back(
        "500003-1,C0,500003,1,Success,16,16,0,0,0,0,0,0,0,0,0,256,3,3");
}

// The events.csv heading and run 500000-2's rows.
void addEventCsvLines(std::vector<std::string>& lines) {
    lines.push_back("");
    lines.push_back("events.csv:");
    lines.push_back(
        "event_id,condition,reasoner,layout_seed,run_seed,type,target,common_"
        "mode,cell,direction,order,action,rotation,scheduled_step,reported_as,"
        "activation_step,activation_delay,not_activated_reason,opportunity,"
        "opportunity_step,revealing_opportunity,mismatch_detected,mismatch_"
        "step,strictly_detected,strict_step,detection_delay,end_state,end_step,"
        "correction_status,verified_correction");
    lines.push_back(
        "500000-2-1,C1,R,500000,2,F1,R1,0,1:6,blocked->free,,,,2,Activated at "
        "its scheduled step,2,0,,0,,0,0,,0,,,Active at end of run,,,0");
    lines.push_back(
        "500000-2-2,C1,R,500000,2,F2,R2,0,,,North South East "
        "West,,,9,Activated at its scheduled "
        "step,9,0,,1,9,1,1,10,1,10,1,Corrected,10,CONSISTENT,0");
    lines.push_back(
        "500000-2-3,C1,R,500000,2,F4,R2,0,7:1,,,,,10,Activated at its "
        "scheduled step,10,0,,0,,0,0,,0,,,Active at end of run,,,0");
    lines.push_back(
        "500000-2-1,C0,R,500000,2,F1,R1,0,1:6,blocked->free,,,,2,Activated at "
        "its scheduled step,2,0,,0,,0,0,,0,,,Overwritten without "
        "correction,2,,0");
    lines.push_back(
        "500000-2-2,C0,R,500000,2,F2,R2,0,,,North South East "
        "West,,,9,Activated at its scheduled step,9,0,,0,,0,0,,0,,,Overwritten "
        "without correction,10,,0");
    lines.push_back(
        "500000-2-3,C0,R,500000,2,F4,R2,0,7:1,,,,,10,Activated at its "
        "scheduled step,10,0,,0,,0,0,,0,,,Active at end of run,,,0");
}

// The events.csv rows of run 500003-1 and the closing line.
void addSecondEventCsvLines(std::vector<std::string>& lines) {
    lines.push_back(
        "500003-1-1,C1,R,500003,1,F5,R1,0,,,,,,1,Activated after "
        "deferral,9,8,,1,9,1,1,10,1,10,1,Corrected,10,VERIFIED,1");
    lines.push_back(
        "500003-1-2,C1,R,500003,1,F4,R1,0,1:7,,,,,2,Activated at its scheduled "
        "step,2,0,,0,,0,0,,0,,,Active at end of run,,,0");
    lines.push_back(
        "500003-1-3,C1,R,500003,1,F1,R2,0,6:8,free->blocked,,,,8,Activated at "
        "its scheduled step,8,0,,0,,0,0,,0,,,Active at end of run,,,0");
    lines.push_back(
        "500003-1-1,C0,R,500003,1,F5,R1,0,,,,,,1,Activated after "
        "deferral,11,10,,0,,0,0,,0,,,Overwritten without correction,11,,0");
    lines.push_back(
        "500003-1-2,C0,R,500003,1,F4,R1,0,1:7,,,,,2,Activated at its scheduled "
        "step,2,0,,0,,0,0,,0,,,Active at end of run,,,0");
    lines.push_back(
        "500003-1-3,C0,R,500003,1,F1,R2,0,6:8,free->blocked,,,,8,Activated at "
        "its scheduled step,8,0,,0,,0,0,,0,,,Overwritten without "
        "correction,8,,0");
    lines.push_back("");
    lines.push_back(
        "These lines show the instrument working; they are not results and are "
        "never compared.");
}

}  // namespace

std::vector<std::string> expectedFaultDemoLines() {
    std::vector<std::string> lines;
    addOpeningLines(lines);
    addEventListLines(lines);
    addRunLines(lines);
    addSecondRunLines(lines);
    addRunCsvLines(lines);
    addEventCsvLines(lines);
    addSecondEventCsvLines(lines);
    return lines;
}

}  // namespace aih
