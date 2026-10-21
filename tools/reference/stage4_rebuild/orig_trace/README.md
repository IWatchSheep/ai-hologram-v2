# The original-code golden tools (Stage 4 rebuild, plan STAGE_04 3.15)

## The original files
- **`original_src/experiment/`** (24 files) and **`original_src/demo/fault_demo.*`** (2) are copied **byte for byte** from the original repository `IWatchSheep/ai-hologram` at commit `89ea9b6` (Stage 4 code-complete). Every hash was verified against `git show 89ea9b6:src/<path>`, and is listed in `docs/PROVENANCE.md` and `docs/PROVENANCE.sha256`.
- The agent files are the Stage 3 copies (`../../stage3/orig_trace/original_src/` with `original_src_ie1/` first: `src/agent` at `89ea9b6`); the world, model and compare files are the Stage 1 and 2 copies. All are identical to `89ea9b6`.
- These files are reference material only. They are never built into the project, are exempt from R1, and are never edited.

## The tools (Linux, g++; `sh build.sh`)
- **`orig_fault_trace.cpp`** reuses the Stage 3 tool (`../../stage3/orig_trace/orig_c1_trace.cpp`, included with its `main` renamed: the case reader, the agent builders, the C1 step printer) and adds the run seeds, the fault events, its own copy of the original `runWithFaults` loop with a print after every decision and after the final check (the Stage 3 lines, the activation records and event scores that changed), and at the end the result line, the CSV rows and the observed evaluations. For every case it then runs the original `runWithFaults` itself and stops with an error unless the outcome, the counts, every line, every activation record, every event score, the run score and the observed evaluations are identical.
- **`orig_material.cpp`** prints, from the original generator, the layouts and event lists of the demonstration experiment (`demo`) and of the 30-run demonstration-seed set (`seeds`). `gen_fault_cases.py` writes those events from the carried-over Python reference; Claude checked that both give identical text.

## How the golden files were produced (2026-10-21)
```
python3 ../gen_fault_cases.py cases <scratch>     # the 563 cases (seed 20261023)
sh build.sh <tools folder>                        # builds the tool and checks:
#   orig_fault_trace traces fault_cases.txt  == ../golden/fault_traces.txt
```
The reference produced a byte-identical file with `python3 ../gen_fault_cases.py traces <dir>`.

The 8 F1-then-F5 cases (`WALL_F5_PICKS` in `gen_fault_cases.py`) were chosen by running this tool on candidates 1–1000 of their own stream: they are the first candidates (C1 and C0) that reach A3's wall check, which the other families reach too rarely.
