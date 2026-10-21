# Stage 3 reference (rules R9, R10, R11, R20)

Claude's Python reference for the rebuilt C1 and C0, built and run before the plan `docs/stages/STAGE_03.md` was reviewed. It follows the original C++ (`src/agent` at `89ea9b6` = `stage-3` plus the IE-1 hooks) **function by function**, so every evaluation, dead-status read and heuristic value happens at the same moment as in the original. It uses `../stage1/` (the world and the reader helpers), `../stage2/` (the budget counter and the world model) and, only for the ten demonstration-seed layouts, `../stage4/ref_stage4.py`.

| File | What it is |
|---|---|
| `ref_parts.py` | Footprint, dead squares and their cache, heuristic, step predictor, agent action model and planner (plan 3.2–3.6), with the IE-1 H3 and H4 reports |
| `ref_verify.py` | Observation, records, investigation, the consequence check, the history checks, model V1 and verification (3.7, 3.8) |
| `ref_agents.py` | The log text, C1, C0 and the run loop with its watchers (3.9–3.12) |
| `ref_trace.py` | The trace cases, the model catalogue, and the trace and observer text of the gate (3.14) |
| `ref_trace_reader.py` | The bounds-safe readers of the gate: the exact order of checks and problem messages (3.14) |
| `checks.py` | The plan's 163 new checks, mirrored with their C++ ids: `failing: none (163 checks)` (`--fast` skips G4 and G5). Recorded output: `checks_output_2026-10-21.txt` |
| `df_check.py` | The deliberate failure checks, tried on the reference first. Recorded output: `df_check_output_2026-10-21.txt` |
| `gen_golden.py` | Writes the 789 cases (`cases` mode), and computes the trace and observer files from this reference (`traces`, `observers`) |
| `golden/` | **The golden files.** `c1_cases.txt` holds the inputs. `c1_traces.txt` and `c1_observers.txt` were **produced by the original C++** (`orig_trace/`). The reference reproduces all three byte for byte. The SHA-256 values are in `docs/PROVENANCE.sha256` |
| `orig_trace/` | The original-code tools, their build script, and the 49 original files they are built from (see its README) |
| `expected_program_output.txt` | The program's exact Stage 3 output (102 lines, 94 non-blank) |

Run the scripts from this folder:
- `python3 checks.py`, which takes about half a minute;
- `python3 df_check.py`, which takes about a minute;
- `python3 gen_golden.py cases <dir>`.

The coding assistant never edits anything here.
