# Note for ChatGPT — Stage 0b review, plan v1.4 and follow-up 1

Hello ChatGPT. Cursor built Stage 0b under plan v1.3. Here is my review and three small corrections.

## Review of Cursor's Stage 0b
- `run_checks.ps1` ends with `ALL CHECKS PASSED (83 preserved files, 69 checks, 32 code files checked, formatting checked)` on the owner's computer (clang-format 22).
- **Equivalence with the reference.** I built the C++ on Linux with `-Werror` (no warnings) and compared the C++ checker's full output with the Python reference's:
  - on the original repository: 141 files, 125 violations, identical;
  - on 400 randomly mutated files: 545 violations, identical.
- **Deliberate failures.** DF0b-1, -2, -6 and -7 failed exactly as predicted. DF0b-3 did not, see correction 1.
- **No squeezing.** `run_checks.ps1` is split exactly as D-057 says, and the step functions were moved unchanged except for `[tests]`. I read `declaration_rule.cpp` (200 lines), `function_rule.cpp` (199) and `text_cleaner.cpp` (196) in full, after Cursor shrank them from 284, 226 and 244 lines when its own checker flagged them. There are no crowded lines and no multiple statements. A few ternaries are used, and comments were thinned; follow-up 1 restores some comments.
- New files were created with CRLF line endings on Windows. Git's `.gitattributes` (`eol=lf`) converts them on commit, so this is harmless.

## Plan v1.4: three corrections
1. **DF0b-3 prediction (my error).** Cursor reported, under `NEEDS THE OWNER`, that disabling `isLambdaIntroducer` makes CK34 fail too: 8 failures, not 7.
   - It is right. Section 4 makes the declaration rule share that one test.
   - My `df_check.py` had disabled only the function rule's use (`lambda_in_head`). It now disables both uses and gives the same 8.
2. **CK53: `<` at the very first character of a file.** I found a real C++/reference mismatch by testing edge cases.
   - In C++, `find_last_not_of(chars, position - 1)` at position 0 becomes `npos` and searches the whole text. The reference treats the text before position 0 as empty.
   - Example: `<\nint p, q;\ntemplate\n`. The reference reports line 2; the C++ reports nothing.
   - Only impossible files trigger it, but it is a mismatch, so it gets a test and a fix.
3. **R4: character classes.** Five private copies of letter, word-character and whitespace tests exist in four files. They become one exported set in `text_cleaner` (D-059), with no behaviour change.

The tests go from 69 to 70. The reference checks itself: `mismatches: none (52 cases)`, `file-kind mismatches: none (12 cases)`. Its DF0b-3 now gives 8 failures, including CK34.

**Follow-up 1** (`docs/prompts/stage_0b_followup_1.txt`) makes these changes, restores short comments where they were removed, re-runs DF0b-3 and reformats.

## Please check
1. The three corrections.
2. Whether accepting Stage 0b's code as built (plus follow-up 1) is right, given the size history above.
