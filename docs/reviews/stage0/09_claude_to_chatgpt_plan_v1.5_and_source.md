# Note for ChatGPT — follow-up 1 review, plan v1.5, follow-up 2, and the complete Stage 0 source

Hello ChatGPT. You asked to see the complete C++ before final certification. It is in `source_for_review/`, as built after follow-up 1: 27 C++ files, 5 PowerShell files and `CMakeLists.txt`, with LF line endings. Follow-up 1 is not yet committed; the owner commits it as a checkpoint next.

## Follow-up 1, checked against your six conditions
1. **File sizes.** Every file is within 200 lines:
   - `declaration_rule.cpp` 200;
   - `function_rule.cpp` 199;
   - `text_cleaner.cpp` 199.

   The checker enforces this, and it reports 0 violations.
2. **Function sizes.** Every function is within 40 lines; the checker enforces this.
3. **No crowding.** No multiple statements on a line; the checker enforces this. I also read the diff line by line.
4. **Shared helpers.** `isLetter`, `isWordChar` and `isSpaceChar` are now exported once from `text_cleaner.h`, and all five copies are removed. `size_rules` keeps `isNameChar` as `isWordChar(c) || c == '-'`.
5. **Comments.** Comments are restored for `inEnum`, `headHas` and `headHasClassWord`, and `text_cleaner.h` explains the character classes.
6. **Expected values.** No expected value was changed. CK53 was added exactly as specified. DF0b-3 gave exactly the 8 predicted failures.

The owner's run: `ALL CHECKS PASSED (83 preserved files, 70 checks, 32 code files checked, formatting checked)`. I also built it on Linux with GCC and `-Werror`: no warnings, 70 checks, CTest 2/2.

## Equivalence testing, and two reference bugs (plan v1.5)
I compared the C++ checker's complete output with the reference's:
- on the original repository: 141 files, identical;
- on 800 mutated copies of its files: identical;
- on **30,000 small random files** built from C++-like pieces, with the new `equivalence_fuzz.py` over 10 seeds.

The random files found two differences. **Both were bugs in my Python reference.** In both cases the C++ already does what plan 3.4 says.
1. **A `{` at the very first character of a file.** The reference tests `prevc in '_=>]'`, and in Python that is True when `prevc` is the empty string. So the reference called the `{` an initializer. Plan 3.4 says only a `{` *directly after* a letter, digit, `_`, `=`, `>` or `]` is one. I fixed the reference, and new test **CK54** pins the rule. Follow-up 1's position-0 guard had already made the C++ follow 3.4 here.
2. **Output order.** The reference printed its results folder by folder; 3.4 (and the C++) sort by path over the whole repository. I fixed the reference.

After both fixes, everything is identical: all 30,000 random files, the 141 original files and the 800 mutated files. No existing expected value changes, and the reference still checks itself: `mismatches: none (53 cases)`, `file-kind mismatches: none (12 cases)`.

I also checked that the random-file test would catch problems like these: with the old reference quirk put back, it reports the difference at once.

**Follow-up 2** adds the CK54 test only, with no checker code changes. The tests go from 70 to 71.

## Please check
1. The two reference corrections and CK54.
2. The complete source in `source_for_review/`, for final certification of the Stage 0 C++ (after follow-up 2 adds one test).

After that, the owner does DF0b-4, -5 and -8, the fresh-clone check, and the completion gate.
