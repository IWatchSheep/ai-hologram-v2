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

## Stage 0b follow-up 1 (rebuild) — 2026-10-20

What changed: (1) One set of character classes (R4, D-059): `text_cleaner.h` / `.cpp` now export `isLetter(char)`, `isWordChar(char)` and `isSpaceChar(char)` (space, tab, `'\r'`, `'\f'`, `'\v'`), moved out of the unnamed namespace; every private copy was removed and the shared ones are used instead (`architecture_rule.cpp`: `isSpace`, `isWordChar` gone; `declaration_rule.cpp`: `isAlphaNumeric` gone, its letter/digit/`_` tests are now `isWordChar`; `size_rules.cpp`: `isNameChar` is now `isWordChar(c) || c == '-'`, `isBlank` gone and its whitespace tests are now `isSpaceChar`; `text_cleaner.cpp`'s `tokenize` also uses `isSpaceChar`). (2) CK53: both backward `find_last_not_of` searches in `declaration_rule.cpp` (`beforeEndsWithTemplate` and `scanBrace`) now treat position 0 as "nothing before it" instead of wrapping `position - 1` to `std::string::npos` and searching the whole text; CK53 was added to `tests/test_code_checker_declarations.cpp` with the exact text from `snippets.py` (`"<\nint p, q;\ntemplate\n"`) and expected exactly `[(2, STYLE-DECLARATIONS)]`. (3) Short beginner-friendly comments were restored for helpers that had none (`inEnum` in `declaration_rule.cpp`, `headHas` and `headHasClassWord` in `function_rule.cpp`); in `function_rule.cpp` the two separate `(...)` / `[...]` / `]` tests in `onlySpecifiers` were combined into one test so the file stays within the limit without squeezing. No behaviour changed apart from the CK53 fix, no expected value was changed, no new files were created, and no Stage 3-baseline files were touched (none exist yet).

Files changed, with line counts (all lines, blank ones included, after clang-format): `tools/code_checker/text_cleaner.h` (66), `tools/code_checker/text_cleaner.cpp` (199), `tools/code_checker/architecture_rule.cpp` (119), `tools/code_checker/declaration_rule.cpp` (200), `tools/code_checker/size_rules.cpp` (126), `tools/code_checker/function_rule.cpp` (199), `tests/test_code_checker_declarations.cpp` (114), and this file (this note appended only). CLion's clang-format was run with `-i` on every file printed by `cmake-build-checks\code_checker.exe --list-cpp .` (27 files) and on nothing else.

Deliberate failure check: DF0b-3 was run again (`isLambdaIntroducer` always returns false, then undone exactly). The test program reported `70 checks, 8 failures`, failing exactly CK29, CK31, CK32, CK33, CK34, CK50, CK51 and CK52, as plan v1.4 predicts.

What was run: `powershell -ExecutionPolicy Bypass -File .\run_checks.ps1` from the repository root, after the deliberate failure was undone. Its exact final line was:

`ALL CHECKS PASSED (83 preserved files, 70 checks, 32 code files checked, formatting checked)`

The run showed `70 checks, 0 failures`, CTest 2/2, `code checks: 32 files, 0 violations`, `[format] clang-format 22.0.0git`, and `program output: 2 lines, 2 non-blank`. No Git operation other than the `git status` inside `run_checks.ps1` was run, and nothing was committed.

Unsure about: `declaration_rule.cpp` (200) and `function_rule.cpp` / `text_cleaner.cpp` (199) are at or near the 200-line limit again, even after Change 1 freed lines; they fit cleanly, but any later change to those files will have almost no room and may need a plan-approved split rather than squeezing.

## Stage 0b follow-up 2 (rebuild) — 2026-10-20

What changed: CK54 only. In `tests/test_code_checker_declarations.cpp`, next to CK53 in `checkFirstCharacter`, one check was added with the exact text from `tools/reference/stage0/snippets.py` (`"{\n    int a, b;\n}\nint x\n"`), path `src/a.cpp`, kind Cpp, expected exactly `[(2, STYLE-DECLARATIONS)]`, description starting `CK54:`. No file in `tools/code_checker/` was changed, no expected value was changed, and no Stage 3-baseline files were touched (none exist yet). The file is 117 lines (all lines, after clang-format). CLion's clang-format was run with `-i` on that one test file only.

What was run: `powershell -ExecutionPolicy Bypass -File .\run_checks.ps1` from the repository root. Its exact final line was:

`ALL CHECKS PASSED (83 preserved files, 71 checks, 32 code files checked, formatting checked)`

The run showed `71 checks, 0 failures`, CTest 2/2, `code checks: 32 files, 0 violations`, `[format] clang-format 22.0.0git`, and `program output: 2 lines, 2 non-blank`. No Git operation other than the `git status` inside `run_checks.ps1` was run, and nothing was committed.

Unsure about: nothing. CK54 passed as plan v1.5 predicts.

## Stage 1a (rebuild) — 2026-10-20

What was built: Stage 1a only (plan Section 7, step 2). The world value types and text format: `Position`, `Action` with `actionName` / `offsetFor` / `moved`, `WorldState` with `gridSize`, `operator==` / `!=`, `isInsideGrid`, `isWall` (outside the grid is a wall, walls read only after a bounds check), `isValidWorld` and `isGoalReached`, and `ParseResult` with `parseWorld` (the original's checks in the original's order, with its exact messages) and `renderWorld` (wall, `+`, `*`, `A`, `O`, `D`, floor), all in namespace `aih`. The test helpers `test_layouts` (DEMO, STAR, PLUS, OPEN, `withLine`, `at`) and `golden_files` (`readLines`), the tests T1–T20, RT1–RT3, V1–V5, V6a–d, N1 and G2 (all 1,009 golden parse cases through `AIH_SOURCE_DIR`, D-063), and the CMake entries and test-suite wiring for them. No rules, environment or demo was created; `currentStage()` still returns 0 and the program output is unchanged. No Stage 3-baseline files were touched (none exist yet), no expected value was changed, and no Git operation other than the `git status` inside `run_checks.ps1` was run; nothing was committed.

Files created, with line counts (all lines): `src/world/position.h` (37), `src/world/action.h` (34), `src/world/action.cpp` (60), `src/world/world_state.h` (57), `src/world/world_state.cpp` (82), `src/world/world_text.h` (42), `src/world/world_parse.cpp` (159), `src/world/world_render.cpp` (53), `tests/test_layouts.h` (46), `tests/test_layouts.cpp` (64), `tests/golden_files.h` (25), `tests/golden_files.cpp` (35), `tests/test_world_parse.cpp` (157), `tests/test_world_render.cpp` (42), `tests/test_world_state.cpp` (79), `tests/test_world_golden.cpp` (135).

Files changed, with line counts (all lines): `CMakeLists.txt` (97, adds the four `src/world` sources to `ai_hologram_core`, the six new test files to `ai_hologram_tests`, and the `AIH_SOURCE_DIR` compile definition), `tests/test_suites.h` (65, four new group declarations), `tests/test_main.cpp` (62, four new group calls), and this file (90, this note appended only). CLion's clang-format was run with `-i` on every new or changed C++ file and on nothing else. One code-checker violation was fixed during the work: `applyCellCharacter` in `world_parse.cpp` was 45 lines with braces on every switch case, so the case braces (not required by the checker, and absent in the original) were removed, bringing it under 40 lines without squeezing.

Deliberate failure checks, each made once, built, run with `cmake-build-checks\ai_hologram_tests.exe`, then undone exactly: DF1a-1 (removed `y == gridSize - 1` from the border-cell test in `world_parse.cpp`) failed exactly T8 and G2 (`FAIL: T8: line 9 with a floor border cell gives the border message` and `FAIL: G2: case 39 expected [ERROR Every border cell must be a wall (#).] actual [ERROR A world must contain exactly one agent (A or +).]`; `105 checks, 2 failures`). DF1a-2 (made `isWall` return false outside the grid in `world_state.cpp`) failed exactly V6a, V6b, V6c and V6d (`105 checks, 4 failures`).

What was run: `powershell -ExecutionPolicy Bypass -File .\run_checks.ps1` from the repository root, before the deliberate failures, after each was undone, and again at the end. Its exact final line was:

`ALL CHECKS PASSED (98 preserved files, 105 checks, 48 code files checked, formatting checked)`

The final run showed `105 checks, 0 failures`, CTest 2/2, `code checks: 48 files, 0 violations`, `[format] clang-format 22.0.0git`, and `program output: 2 lines, 2 non-blank`.

Unsure about: nothing in the behaviour. Two small judgement calls, both following the plan: `Position`'s operators are defined in `position.h` because the plan names no `position.cpp`, and the switch cases in `applyCellCharacter` and `actionName` carry no braces, matching the original source, because the braces rule (R2) names `if`/`else`/`for`/`while`/`do` bodies.

## Stage 1b (rebuild) — 2026-10-20

What was built: Stage 1b only (plan Section 7, step 3). The movement rules `applyAction` (validity, wall at target, plain move, wall beyond the object, push — in that order; a `const WorldState&` in and a new `ActionResult` out, the input never changed) with `MoveOutcome` / `outcomeName`, and `Environment` (private state copied by the constructor, `step` storing the result and adding one to the count every time, `state() const`, `stepCount() const`). The shared bounds-safe golden readers behind `golden_readers.h`: the parse reader moved out of `test_world_golden.cpp`, and the new rules reader, both following `ref_golden_readers.py` exactly (same order of checks, same problem messages, line existence checked before every access, no exceptions), plus `parseInteger` in `golden_files` (optional `-`, then digits only). Tests R1a–d, R2–R7, R8a–c, R9, R10, R11a–b, N2, E1–E4, G1 (5,184 cases), GN1–GN6 and GR1–GR5; G2 now calls the shared parse reader and its result is unchanged. No demo was created, `currentStage()` and the program output are unchanged (Stage 1c), no expected value was changed, no Stage 3-baseline files were touched (none exist yet), and no Git operation other than the `git status` inside `run_checks.ps1` was run; nothing was committed.

Files created, with line counts (all lines): `src/world/rules.h` (44), `src/world/rules.cpp` (67), `src/world/environment.h` (50), `src/world/environment.cpp` (38), `tests/test_rules.cpp` (150), `tests/test_environment.cpp` (84), `tests/test_world_golden_rules.cpp` (106), `tests/golden_readers.h` (40), `tests/golden_parse_reader.cpp` (157), `tests/golden_rules_reader.cpp` (194).

Files changed, with line counts (all lines): `CMakeLists.txt` (104, adds `environment.cpp` / `rules.cpp` to `ai_hologram_core` and `golden_parse_reader.cpp` / `golden_rules_reader.cpp` / `test_environment.cpp` / `test_rules.cpp` / `test_world_golden_rules.cpp` to `ai_hologram_tests`), `tests/test_suites.h` (74, three new group declarations), `tests/test_main.cpp` (65, three new group calls), `tests/test_world_golden.cpp` (103, G2 via the shared reader plus GN1–GN6), `tests/golden_files.h` (32, `parseInteger` declared), `tests/golden_files.cpp` (68, `parseInteger` defined), and this file (this note appended only). CLion's clang-format was run with `-i` on every new or changed C++ file and on nothing else. During the work `golden_rules_reader.cpp` first came to 201 lines; it was brought to 194 without squeezing by having its action-name lookup compare against the shared `actionName` instead of repeating the four name tests.

Deliberate failure checks, each made once, built, run with `cmake-build-checks\ai_hologram_tests.exe`, then undone exactly: DF1b-1 (a push into a wall returns the state with the agent moved, outcome `Blocked`) failed exactly R4 and G1 (`FAIL: R4: East push into the border wall is Blocked and unchanged` and `FAIL: G1: line 326: expected [L1 2 1 1 1 West -> Blocked 2 1 1 1 0] actual [L1 2 1 1 1 West -> Blocked 1 1 1 1 0]`; `139 checks, 2 failures`). DF1b-2 (the parse reader's `expect ` prefix check removed) failed exactly GN3 (`FAIL: GN3: expect line lacks the expect prefix`; `139 checks, 1 failures`). For DF1b-2, removing only the check first left its `startsWith` helper unused, which the warnings-as-errors build rejects, so the helper was removed with the check for that run and both were restored exactly afterwards.

What was run: `powershell -ExecutionPolicy Bypass -File .\run_checks.ps1` from the repository root, before the deliberate failures, after the line-count fix, and again at the end after both deliberate failures were undone. Its exact final line was:

`ALL CHECKS PASSED (98 preserved files, 139 checks, 58 code files checked, formatting checked)`

The final run showed `139 checks, 0 failures`, CTest 2/2, `code checks: 58 files, 0 violations`, `[format] clang-format 22.0.0git`, and `program output: 2 lines, 2 non-blank`.

Unsure about: only the DF1b-2 detail above — removing the `expect ` prefix check on its own also orphans the `startsWith` helper in `golden_parse_reader.cpp`, so a build with warnings as errors stops before the test program can report GN3; with the orphaned helper removed alongside, the run fails exactly GN3 as predicted. Claude and the owner may want the plan's DF1b-2 wording to name both removals. Nothing else.
