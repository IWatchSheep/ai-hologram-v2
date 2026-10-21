# Handoff notes from the coding assistant

After each piece of work, the coding assistant appends one note here (AGENTS.md, rule 9): what it built, what it ran, the exact `run_checks.ps1` result, and anything it was unsure about. Earlier notes are never changed. The notes from the original repository are kept in `docs/history/ai-hologram/results/notes_from_github_copilot/`.

## Stage 0a (rebuild) — 2026-10-20

What was built: the CMake project (core library, program, test program, one CTest test), `aih::projectName()` / `aih::currentStage()`, the two-line program output, the quiet-by-default test runner, tests P1, P2, TS1, TS2, TS3, and `run_checks.ps1` steps `[path]` (now failing if the MinGW or CMake folder is missing), `[provenance]`, `[configure]`, `[build]`, `[tests]`, `[ctest]`, `[program]` and `[git]`, with every external program run through the one `Invoke-Tool` helper. No code checker targets or `[code]` / `[format]` steps: those are Stage 0b.

Files created, with line counts: `CMakeLists.txt` (55), `src/core/project_info.h` (22), `src/core/project_info.cpp` (20), `src/main.cpp` (15), `tests/test_support.h` (33), `tests/test_support.cpp` (24), `tests/test_suites.h` (20), `tests/test_main.cpp` (27), `tests/test_project_info.cpp` (65).

Files changed: `run_checks.ps1` (193 lines), `README.md` (status line, plan Section 9, plus the following sentence updated so it no longer says no C++ code exists), and this file (this note appended only).

What was run: `powershell -ExecutionPolicy Bypass -File .\run_checks.ps1` from the repository root (twice; the second run was after trimming `run_checks.ps1` under 200 lines). Its exact final line was:

`ALL CHECKS PASSED FOR STAGE 0a (83 preserved files, 5 checks; code and format checks are added in Stage 0b)`

Program output was exactly:

```
AI Hologram v2 -- Stage 0
Project skeleton: build, tests and code checks only. No world, agents or experiments yet.
```

`cmake-build-checks\ai_hologram_tests.exe --verbose` (with CLion's MinGW on PATH, as `run_checks.ps1` sets it) printed the `--- Project info ---` heading, PASS lines for P1, P2, TS1, TS2 and TS3, and `5 checks, 0 failures`. CTest reported 1/1 tests passed. No Stage 3-baseline files were touched (none exist yet). No Git operation other than status was run, and nothing was committed.

Unsure about: the README status line follows plan Section 9's wording, which names the code checker even though it is built in Stage 0b; the same sentence in the README says the code checker follows in Stage 0b, so the status is not overstated.
