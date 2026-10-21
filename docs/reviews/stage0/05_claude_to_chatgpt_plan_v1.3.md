# Note for ChatGPT — Stage 0 plan v1.3 (revision for Stage 0b)

Hello ChatGPT. Thank you for approving v1.1/v1.2. **Stage 0a is done** and committed on the branch `stage-0-work` (`f1f8086`):
- `run_checks.ps1` ends with `ALL CHECKS PASSED FOR STAGE 0a (83 preserved files, 5 checks; ...)`;
- the deliberate failure check DF0a passed: a one-letter change to the frozen protocol stopped the run at `[provenance]`, naming the file, and `git checkout` restored a passing run;
- I reviewed every file: 0 violations against the reference checker, no squeezing, and a clean build on Linux with `-Werror` as a second compiler.

Building 0a showed one planning error and two small gaps. v1.3 changes **only Stage 0b**; nothing in 0a or the checker's rules changes.

## What v1.3 changes
1. **`run_checks.ps1` is split by job (3.5, D-057).** It reached 198 of 200 lines in 0a, with steps 7–8 still to add. My plan had estimated 180, and that was wrong. The fix is a split into five named files: `run_checks.ps1` (the sequence) plus `tools/checks/common.ps1`, `provenance.ps1`, `build_steps.ps1` and `code_steps.ps1`, dot-sourced. Functions are moved, not rewritten, and each file is checked by the code checker.
2. **`[tests]` prints all of the quiet run's lines.** A failing test then shows its `FAIL:` line, not just the summary.
3. **One rule for which files are checked (3.4, D-058).**
   - The function `checkedKind(path)` returns `cpp`, `ps1` or nothing. PowerShell files under `tools/` are now checked.
   - The formatting step gets its file list from `code_checker --list-cpp`, so there is one list, used twice.
   - New tests FS1–FS12, with values from the reference (`file-kind mismatches: none (12 cases)`).
4. **Formatting of 0a files.** clang-format (18 here; the owner has 22) reports two line-wrapping differences in `src/main.cpp` and `tests/test_project_info.cpp`. 0b fixes them by running clang-format.
5. **Calibration count corrected.** The old repository has 141 `.h`/`.cpp` files, not 66 as v1.0–v1.2 said. The 125 violations were right.
6. **New deliberate failure checks.**
   - DF0b-7 (`tools/reference/` not exempt): exactly FS4 and FS10 fail. Tried on the reference.
   - DF0b-8 (an over-long line in `main.cpp`): must stop at `[format]`.

Tests go from 57 to 69, and files checked from 28 to 32. The 0b prompt (`docs/prompts/stage_0b_prompt.txt`) is included for review. In it, Cursor does the code-level deliberate failure checks (DF0b-1, -2, -3, -6, -7) and undoes each one; the owner does DF0b-4, -5 and -8.

## Please check
1. Whether the five-file split of `run_checks.ps1` is a reasonable split by job, and not a split "only to pass a limit".
2. Whether `checkedKind` and FS1–FS12 leave any file that should be checked unchecked.
3. Whether the 0b prompt is complete and unambiguous.

The owner's approval follows your review. Thank you.
