# Note for ChatGPT: Stage 4 plan (rebuild) v1.0, fault injection and measurement, and the rest of the equivalence gate

Hello ChatGPT. Stage 3 of the rebuild is complete and tagged `stage-3-v2` (`ec4613f`), after your final approval of `336d4d9`. This is the Stage 4 plan. As before, the reference and the golden values were built first.

**The package is the whole buildable repository at `main` (`ec4613f`), plus the new Stage 4 files:**
- `docs/stages/STAGE_04.md`;
- `docs/prompts/stage_4a_prompt.txt`;
- decisions D-073 to D-080 in `docs/DECISIONS.md` (marked "proposed");
- the provenance update (163 → 191 preserved files);
- `tools/reference/stage4_rebuild/` (the carried-over original Stage 4 reference in `tools/reference/stage4/` is unchanged).

## What Stage 4 rebuilds
- The original's Stage 4 (`src/experiment/` and `src/demo/fault_demo.*` at `89ea9b6`), **exactly**: SplitMix64, the layout generator, the fault-event lists, the five fault types with A3 and A4, the activation records, the scorer (phases, claims, the event life, the committed-prediction shadow, the observed shadow), CSV rows, and the demonstration text.
- **The original readings D-030 to D-045 are carried over unchanged** (D-073). Nothing about the measures is re-decided.
- **IE-1 needs nothing new**: H1–H4 were built into the Stage 3 agents (R5).

## The equivalence gate, part 2 (plan 3.15, D-074)
- **The tool** `orig_trace/orig_fault_trace.cpp` is compiled only from the original C++ at `89ea9b6` (`src/agent`, `src/experiment`, and the Stage 1–2 originals). It reuses the Stage 3 tool and adds the events. For each case it prints, after every decision step: the Stage 3 trace lines, every activation record that changed, and every event score that changed; at the end, the result line, the `runs.csv` row, one `events.csv` row per event, and the observed evaluations. It then reruns the case with the **original `runWithFaults`** and stops unless every result, line, record and score is identical.
- **The 563 cases** cover M1.4's list and more:
  - FR1 and the four FR4 runs;
  - the 16 demonstration-experiment runs;
  - **the 60 further demonstration-seed runs** (500000–500009 × run seeds 1–3, C1 and C0), with the event list of that 30-run set; the same layouts without faults are Stage 3's 20 demonstration-seed cases;
  - 482 synthetic fixtures (D-070, seed 20261023), including cases the experiment generator never makes (several events at one step, late steps, repeated F3 on one action, events without a cell). Between them the cases reach every reason text, every end state and every correction status, trap claims, wrong accepted corrections, false alarms and mismatches without a fault cause.
- **The reference reproduces all 563 traces byte for byte**, and the program output (170 lines).
- **G8** requires exactly 563 cases (your review 04 of Stage 3); **G9/G10** prove the count is enforced.
- **With G4–G10 passing, M1.4 has passed in full**, and `c1-baseline-v2` is tagged on Stage 4's completion commit (D-079).

## Points I would like you to review
1. **`onObservation` (3.6, 3.12, D-075).** The original wrote the run loop a second time inside `fault_run.cpp`. I propose that the run with faults reuses the Stage 3 loop, with one more watch-only watcher that receives every observation just before the agent does (for `decide` and for the final check), so the fault run can keep the true history. A watcher after the decision is not enough: during the final check the observer already needs the final observation in the history. This is the only change to `src/agent` behaviour code paths; it is watch-only, and G4–G10 prove nothing changes.
2. **`actionIndex` moved** to `world/action` (D-076, R4).
3. **Tests not repeated (D-077).** The original's per-run tables for the demonstration experiment (FR2, FR3, SC5, SH5, CSV2; about 90 checks) are not separate tests: the gate compares every one of their fields, at every step, for the same 16 runs, and FD1 compares CSV2's rows. All other original checks are kept with their ids (LG renamed LY). 195 new checks; 537 in total.
4. **Quirk Q7 (D-078).** The original's CSV text `Not activated: deferred, never applicable` contains a comma, and the fields are never quoted. The original plan said no field contains one. Keeping it is required by M1.3 item 5; later readers must count columns. Do you agree this is a quirk to keep rather than a bug to report under Phase A 3?
5. **Material-only checks (D-080).** LY5, FE2 and FE3 generate the pilot and confirmatory layouts and event lists (as the original did) to check the protocol's balance. No agent is run on them. Is this acceptable under R21 as clarified by D-070?
6. **`c1-baseline-v2` at Stage 4's completion (D-079)**, on the same commit as `stage-4-v2`.
7. The deliberate failure checks (Section 7), one per sub-stage plus the owner's two, all tried on the reference first (`df_check_output_2026-10-21.txt`).

Thank you.
