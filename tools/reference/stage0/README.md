# Stage 0 reference (rule R9, R10)

Claude's Python reference for the Stage 0 code checker, built and run **before** the plan was written. Every expected value in `docs/stages/STAGE_00.md` Section 6.2 comes from these files.

| File | What it is |
|---|---|
| `ref_code_checker.py` | The checker's rules, exactly as plan Section 3.4 describes them. `python3 ref_code_checker.py <folder>` checks a repository folder, choosing files with `checked_kind` (plan 3.4), and prints `code checks: <files> files, <violations> violations`. Run the scripts from this folder. |
| `snippets.py` | CK1–CK27 and CK29–CK54 (CK28 is a formatting test in C++ only): the snippet texts and their expected (line, rule) lists. Running it prints each result and `mismatches: none (53 cases)`; it also holds FS1–FS12 (which files are checked) and prints `file-kind mismatches: none (12 cases)`. |
| `equivalence_fuzz.py` | Compares the built C++ checker with this reference on 3,000 random small files per seed: `python3 equivalence_fuzz.py <code_checker> <scratch folder> [seeds]`. Recorded run: `equivalence_fuzz_output_2026-10-20.txt` (10 seeds, all identical). |
| `df_check.py` | The deliberate failure checks DF0b-1, DF0b-2, DF0b-3, DF0b-6 and DF0b-7 tried on the reference first (R11). |
| `snippets_output_2026-10-20.txt`, `df_check_output_2026-10-20.txt` | The recorded outputs. |

Predicted and confirmed on the reference:
- DF0b-1 (function limit 40 → 41): exactly CK3, CK22, CK25, CK29, CK31, CK32, CK33, CK35, CK36, CK37, CK50, CK51, CK52 fail.
- DF0b-2 (the braces rule ignores `else`): exactly CK7 fails.
- DF0b-3 (the one lambda-introducer test always false, so both the function rule and the declaration rule lose lambdas): exactly CK29, CK31, CK32, CK33, CK34, CK50, CK51, CK52 fail. (Corrected in plan v1.4; before, `df_check.py` disabled only the function rule's use and missed CK34.)
- DF0b-6 (no `noexcept(...)`/`decltype(...)` groups in lambda heads): exactly CK51, CK52 fail.
- DF0b-7 (`tools/reference/` no longer exempt): exactly FS4, FS10 fail.

Plan version: STAGE_00 v1.5 (v1.5: two reference bugs fixed, a `{` at position 0 and output order, CK54; v1.4: CK53, DF0b-3 corrected; v1.3: which files are checked, `checked_kind`; v1.1: lambda bodies, lambda-aware declarations, include paths; v1.2: `noexcept(...)` and `decltype(...)` in lambda heads).

Calibration: on the original repository's C++ at `89ea9b6` (141 `.h`/`.cpp` files) the reference finds 125 violations (10 SIZE-FILE, 28 SIZE-FUNCTION, 58 STYLE-BRACES, 29 STYLE-STATEMENTS), all genuine on review.

The coding assistant never edits these files. A disagreement between the C++ and this reference is reported, never "fixed" by changing an expected value (R22).
