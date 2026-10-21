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

## Stage 0b (rebuild) — 2026-10-20

What was built: the code checker as a faithful C++ port of `tools/reference/stage0/ref_code_checker.py` (cleaning and tokens and `findMatching` only in `text_cleaner`; `isLambdaIntroducer` only in `function_rule`, called by the declaration rule; `checkedKind` and `checkText` in `checker`), the `code_checker` program with `--list-cpp`, the CTest test `code_checks`, checker tests CK1–CK52 and FS1–FS12 (69 checks in total with P/TS), the split of `run_checks.ps1` into itself plus `tools/checks/common.ps1`, `provenance.ps1`, `build_steps.ps1` and `code_steps.ps1` (moving the Stage 0a functions, with only the step 5 change: all quiet test lines are printed), new steps 7 (`[code]`) and 8 (`[format]`, failing when CLion's clang-format is missing), and the full final line. Every file printed by `code_checker.exe --list-cpp .` (27 files) was formatted with CLion's clang-format `-i`, and nothing else; this changed `src/main.cpp` and `tests/test_project_info.cpp` in line wrapping only. No Stage 3-baseline files were touched (none exist yet), no expected value was changed, and no Git operation other than status/diff was run.

Files created, with line counts (all lines, blank ones included): `tools/code_checker/text_cleaner.h` (58), `tools/code_checker/text_cleaner.cpp` (196), `tools/code_checker/size_rules.h` (32), `tools/code_checker/size_rules.cpp` (139), `tools/code_checker/function_rule.h` (28), `tools/code_checker/function_rule.cpp` (199), `tools/code_checker/style_rules.h` (29), `tools/code_checker/style_rules.cpp` (160), `tools/code_checker/declaration_rule.h` (25), `tools/code_checker/declaration_rule.cpp` (200), `tools/code_checker/architecture_rule.h` (26), `tools/code_checker/architecture_rule.cpp` (138), `tools/code_checker/checker.h` (36), `tools/code_checker/checker.cpp` (161), `tools/code_checker/checker_main.cpp` (157), `tests/test_code_checker_size.cpp` (137), `tests/test_code_checker_style.cpp` (96), `tests/test_code_checker_declarations.cpp` (106), `tests/test_code_checker_architecture.cpp` (91), `tools/checks/common.ps1` (32), `tools/checks/provenance.ps1` (41), `tools/checks/build_steps.ps1` (98), `tools/checks/code_steps.ps1` (61).

Files changed, with line counts: `CMakeLists.txt` (82), `run_checks.ps1` (57), `tests/test_support.h` (37), `tests/test_support.cpp` (34, adds `repeatLine`), `tests/test_suites.h` (53), `tests/test_main.cpp` (58), `src/main.cpp` (17, formatting only), `tests/test_project_info.cpp` (67, formatting only), `README.md` (30, status line: Stage 0 complete apart from its gate), and this file (this note appended only).

Deliberate failure checks, each made once, built, run with `cmake-build-checks\ai_hologram_tests.exe`, then undone exactly: DF0b-1 (function limit 40 to 41) failed exactly the 13 predicted checks CK3, CK22, CK25, CK29, CK31, CK32, CK33, CK35, CK36, CK37, CK50, CK51, CK52. DF0b-2 (braces rule ignores `else`) failed exactly CK7. DF0b-6 (lambda heads reject every `(` after the parameter list) failed exactly CK51, CK52. DF0b-7 (`checkedKind` stops exempting `tools/reference/`) failed exactly FS4, FS10. DF0b-3 is the exception, reported under "Unsure about" below. DF0b-4, DF0b-5 and DF0b-8 were not done (the owner's).

What was run: `powershell -ExecutionPolicy Bypass -File .\run_checks.ps1` from the repository root (before formatting, after formatting, and again at the end after every deliberate failure was undone). Its exact final line was:

`ALL CHECKS PASSED (83 preserved files, 69 checks, 32 code files checked, formatting checked)`

The run showed `69 checks, 0 failures`, CTest 2/2, `code checks: 32 files, 0 violations`, `[format] clang-format 22.0.0git`, and `program output: 2 lines, 2 non-blank`.

Unsure about: DF0b-3 (`isLambdaIntroducer` always returns false) failed 8 checks, not the predicted 7: the predicted CK29, CK31, CK32, CK33, CK50, CK51, CK52 plus CK34. This follows from the plan's own design (Section 4): the declaration rule calls the one shared `isLambdaIntroducer`, so disabling it also stops CK34's lambda body being recognised; the Python reference has a separate character-based lambda test for declarations, so its DF0b-3 prediction (which replaces `lambda_in_head`, not `is_lambda_intro`) misses CK34. As evidence, replacing `lambdaInHead` (the reference's exact mutation point) instead gave exactly the predicted 7 failures. No expected value was changed; Claude and the owner should decide whether the plan's DF0b-3 prediction should name CK34 for the C++ checker, or whether the mutation point should be described as `lambdaInHead`. Also: three checker files sit close to the 200-line limit (`declaration_rule.cpp` exactly 200, `function_rule.cpp` 199, `text_cleaner.cpp` 196) because a faithful C++ port with braces on every body is longer than the Python reference; they fit without squeezing, but later changes to those files will have little room.
