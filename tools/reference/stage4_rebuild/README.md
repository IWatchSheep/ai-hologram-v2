# Stage 4 reference (rules R9, R10, R11, R20)

Claude's Python reference for the rebuilt Stage 4 (fault injection and measurement), built and run before the plan `docs/stages/STAGE_04.md` was reviewed. It follows the original C++ (`src/experiment` and `src/demo/fault_demo` at `89ea9b6`) **function by function and draw by draw**, on top of the Stage 3 reference (`../stage3/`: C1, C0, their run loop and trace text) and the Stage 1 and 2 references. The carried-over original Stage 4 reference (`../stage4/`) is kept unchanged as history; this folder does not change it, and uses only its `make_layout` and `make_events` to write the demonstration cases (they equal the original C++ generator, `orig_trace/orig_material.cpp`).

| File | What it is |
|---|---|
| `ref_material.py` | Seeded random numbers, the experimenter's search, the layout generator, the fault-event lists and their text (plan 3.1–3.3) |
| `ref_faults.py` | The faulty model, applying a fault, the activation records, the run loop with the `onObservation` watcher, and the run with faults (3.4–3.6) |
| `ref_scorer.py` | The read-only view, the scorer (phases, claims, the event life, the committed shadow, the observer callbacks) and the shadow functions (3.7–3.9) |
| `ref_fault_csv.py` | The CSV headers and rows (3.11) |
| `ref_fault_demo.py` | The Stage 4 demonstration text and the whole program output (3.14, Section 5) |
| `ref_fault_trace.py` | The fault-trace text of the gate, part 2 (3.15) |
| `ref_fault_reader.py` | The bounds-safe reader of the fault cases and the gate function, with the exact problem messages (3.15) |
| `checks.py` | The plan's 195 new checks, mirrored with their C++ ids: `failing: none (195 checks)` (`--fast` skips the slow material checks and G8). Recorded output: `checks_output_2026-10-21.txt` |
| `df_check.py` | The deliberate failure checks, tried on the reference first. Recorded output: `df_check_output_2026-10-21.txt` |
| `gen_fault_cases.py` | Writes the 563 fault cases (`cases`), and computes the trace file from this reference (`traces`) |
| `golden/` | **The golden files.** `fault_cases.txt` holds the inputs; `fault_traces.txt` was **produced by the original C++** (`orig_trace/`). The reference reproduces it byte for byte. The SHA-256 values are in `docs/PROVENANCE.sha256` |
| `orig_trace/` | The original-code tools, their build script, and the 26 original files they are built from (see its README) |
| `expected_program_output.txt` | The program's exact Stage 4 output (170 lines, 152 non-blank) |

Run the scripts from this folder:
- `python3 checks.py`, which takes about four minutes (`--fast`: about fifteen seconds);
- `python3 df_check.py`, which takes about half an hour;
- `python3 gen_fault_cases.py cases <dir>` (a few seconds) or `traces <dir>` (about two minutes).

The coding assistant never edits anything here, and never runs Python here.
