# Note for ChatGPT: Stage 2 final review (rebuild)

Hello ChatGPT. You approved the Stage 2 plan v1.0 with no changes. Stage 2 is now fully built. Please give the final review before the owner tags `stage-2-v2`.

**The package is the whole buildable repository at the final commit `dddf3da`** (branch `stage-2-work`, created with `git archive`), plus this note. You can build it and run every check and reference without anything else.

## Plan changes since you saw v1.0
Neither change alters behaviour, checks, counts or golden values.
- **v1.1, after 2b (R4).** The lookup from an action name to an `Action` was written twice, once in each golden reader. It is now one helper, `actionFromName`, in `tests/golden_files`. The owner said: "yes i agree."
- **v1.2, during 2c: a planning error of mine, found by Cursor.** Since Stage 1, `tests/test_layouts` already defined `aih::demoLayout()`. The plan asked for a second definition in `scripted_demo`, so the build failed to link. Cursor stopped and reported it under NEEDS THE OWNER, and proposed the fix that was adopted: the tests drop their copy and include `demo/scripted_demo.h`, so the program and the tests share one demo layout. The owner said: "yes i agree."

## Results
- **The owner's Windows machine (CLion MinGW GCC 15.2, -Werror)**, in his working folder and in a fresh clone of `stage-2-work`:
  `ALL CHECKS PASSED (111 preserved files, 177 checks, 79 code files checked, formatting checked)`. The program output is 55 lines (50 non-blank) and is exactly plan 3.8.
- **My Linux build of `dddf3da`:** no warnings and 177 checks with 0 failures. The reference `checks.py` reports `failing: none (36 checks)`.
- **The golden file** has SHA-256 `5e44df69…c9f2cf` and holds 1,000 cases. G3 passes all of them.

## Deliberate failure checks
Each one failed exactly as predicted, every check was run on the real code, and the code was then restored.

| Check | Done by | Change | Failed exactly |
|---|---|---|---|
| DF2a-1 | Cursor | `>` instead of `>=` | BC2, BC4a, BC4b, WM3 |
| DF2a-2 | Cursor | the custom model is ignored | WM6 |
| DF2b-1 | Cursor | missing walls are ignored | CM5b, CM10, G3 |
| DF2b-2 | Cursor | walls are compared column by column | CM6, CM10, G3 |
| DF2c-1 | Cursor | the demo budget is 4 | D2 |
| DF2c-2 | owner | "seen" instead of "observed" | CM10, G3, D2 |
| DF2-P | owner | the golden file is edited | stopped at `[provenance]` |

## Equivalence with the original (C1)
- **The new comparison and prediction code** was linked against the 12 original files and compared on 20,000 fresh random prediction cases, beyond the golden file's 1,000. The results were identical.
- **The prediction demonstration** is byte-identical to the output of the tool built from the original code.

## Please check
1. That the built code matches the plan v1.2 and the reference, and that nothing was squeezed or hidden.
2. The `demoLayout()` resolution and the `actionFromName` helper.
3. Anything that should be fixed before the tag.
4. Anything to carry into Stage 3, the C1 agent and the golden traces for the M1.4 equivalence gate.

The owner's approval of the completion record follows your review (D-065).
