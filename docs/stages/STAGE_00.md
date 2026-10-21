# Stage 0 (rebuild) — Project skeleton, checks and the run script

| | |
|---|---|
| **Project** | AI Hologram v2 (`ai-hologram-v2`) |
| **Stage** | 0 of the rebuild (Brief Section 10, Stage 0: project skeleton) |
| **Plan version** | 1.3 (revision for Stage 0b, after Stage 0a was completed and committed at `f1f8086`) |
| **Date** | October 20, 2026 |
| **Status** | **APPROVED.** v1.2 approved by ChatGPT and the owner (Stage 0a was built under it). v1.3, which changes only Stage 0b, approved by ChatGPT (with the DF0b-4 correction made) and by the owner on 2026-10-20 ("i approve"). Stage 0b code may be written only after v1.3 is committed. |
| **Governing documents** | The brief v1.1; `docs/RULES.md` (R1–R24, approved); `AGENTS.md`. The Experiment 1 protocol is not affected by Stage 0. |
| **Builds on** | The first commit of `ai-hologram-v2` (`0484b6d`): governing documents, provenance, history, references, `run_checks.ps1` (provenance only). |
| **Reference** | `tools/reference/stage0/ref_code_checker.py` and `snippets.py` (R9, R10): the code checker's rules, built and run before this plan, producing every expected value in Section 6.2. `snippets.py` checks itself (`mismatches: none (51 cases)`, `file-kind mismatches: none (12 cases)`). |

---

## 1. Purpose
Stage 0 builds the **frame every later stage relies on**, with the rules of `docs/RULES.md` enforced by code from the first day:
1. a C++17 CMake project with a library, a program and a test program (quiet by default, R15);
2. the **code checker** (R1, R2, R7): file and function sizes, braces, one statement and one declaration per line, no raw strings, and agent code never including experiment code. It has its own tests made of deliberately bad snippets;
3. the **complete `run_checks.ps1`** (R14): PATH, provenance, configure, build (no warnings allowed), tests, CTest, code checks, formatting check, program output counts, `git status`. It stops at the first failure.

**What Stage 0 does not contain:** no grid world, no agents, no randomness, no experiment code. Those are Stages 1–4, each with its own plan.

## 2. What Stage 0 must NOT contain
- No world, rules, agents, models, faults or experiment code.
- No external libraries; C++17 standard library only (`<filesystem>` is allowed for the checker tool).
- No exceptions, no assertions that stop the program, no inheritance.
- No file over 200 lines, and no function over 40 lines, from the first commit (the checker checks itself).

## 3. Concepts and exact behaviour

### 3.1 The CMake project
- `cmake_minimum_required(VERSION 3.20)`; `project(ai_hologram_v2 VERSION 0.0.0 LANGUAGES CXX)`; C++17 required, extensions off.
- Warnings: `-Wall -Wextra -Wpedantic` **and `-Werror`** for GCC and Clang (MSVC: `/W4 /WX`), on every target, controlled by a CMake option `AI_HOLOGRAM_WARNINGS_AS_ERRORS` that is `ON` by default. So any warning stops the build itself, in CLion as well as in `run_checks.ps1`, and the warning text is still shown by the compiler.
- As a second, independent net, `run_checks.ps1` also fails if the build output contains `warning:` (3.5 step 4), and it passes `-DAI_HOLOGRAM_WARNINGS_AS_ERRORS=ON` explicitly, so the option cannot be switched off quietly in its build folder.
- **Targets:**
  - `ai_hologram_core` (static library): `src/core/project_info.cpp`;
  - `ai_hologram` (program): `src/main.cpp`;
  - `code_checker_core` (static library): the checker's rules in `tools/code_checker/` (all files except `checker_main.cpp`);
  - `code_checker` (program): `tools/code_checker/checker_main.cpp`;
  - `ai_hologram_tests` (test program): `tests/*.cpp`, linked to both libraries.
- **CTest:** two tests: `correctness_tests` (runs `ai_hologram_tests`) and `code_checks` (runs `code_checker` on the repository root, `${CMAKE_SOURCE_DIR}`).

### 3.2 The program
`ai_hologram` prints exactly two lines:
```
AI Hologram v2 -- Stage 0
Project skeleton: build, tests and code checks only. No world, agents or experiments yet.
```
The first line is built from `projectName()` ("AI Hologram") and `currentStage()` (0).

### 3.3 The test program (quiet by default, R15)
- `TestCounter` holds `checks`, `failures`, `verbose` (bool) and `std::ostream* out` (default `&std::cout`).
- `check(counter, condition, description)` increments `checks`. On failure it increments `failures` and prints `FAIL: <description>`. On success it prints `PASS: <description>` **only** when `verbose` is true.
- `main` sets `verbose` when the first argument is `--verbose`. Each group prints its heading (`--- <name> ---`) only in verbose mode. The last line is always `<checks> checks, <failures> failures`. The exit code is 0 only with no failures.

### 3.4 The code checker (R1, R2, R7) — exact rules
**Input:** text with LF line endings, and the file's path relative to the repository root (forward slashes).

**Cleaning (C++ files).** Before any style rule, comments and the contents of string and character literals are replaced by spaces, keeping every line break:
- `//` to the end of the line, and `/* ... */` (possibly over several lines), become spaces;
- in `"..."` and `'...'` the quotes stay and everything between becomes spaces. A backslash escapes the next character. A literal ends at its closing quote or at the end of the line.
- `R"` (not preceded by a letter, digit or `_`) marks a raw string: the line is reported as `STYLE-RAW-STRING`, and cleaning treats the `R` as an ordinary letter.

**Tokens.** Identifiers (letters, digits, `_`, starting with a letter or `_`), numbers, and single punctuation characters, each with its line number.

**The rules** (each violation is a line number and a rule name):
| Rule | Violation when |
|---|---|
| `SIZE-FILE` | the file has more than 200 lines (a final newline does not count as a line). Reported at line 1. |
| `SIZE-FUNCTION` | a function body spans more than 40 lines, counting from the line of its `{` to the line of its `}`, inclusive. Reported at the line of the `{`. |
| `STYLE-BRACES` | after `if (...)`, `if constexpr (...)`, `for (...)`, `while (...)` or `switch (...)` (with the matching `)` found by counting parentheses), the next token is not `{`; or after `else` the next token is neither `{` nor `if`; or after `do` the next token is not `{`. **Exception:** a `while (...)` directly preceded by `}` and followed by `;` (the end of a do-while). Reported at the keyword's line. |
| `STYLE-STATEMENTS` | a line contains more than one `;` outside parentheses. Parentheses are counted across lines, so a `for (...)` header's semicolons do not count. |
| `STYLE-DECLARATIONS` | a statement ending in `;` (outside parentheses) contains a comma outside `()`, `[]`, `{}` and template `<>`, and is not inside an `enum` body. Reported at the line of the first such comma. A `<` opens a template only when it directly follows a letter, digit or `_` with no space, or follows the word `template`, and is not `<<` or `<=`; a `>` closes one only when a template is open, and it is not `->` or `>=`. A `{` directly after a letter, digit, `_`, `=`, `>` or `]` starts a brace-initializer (counted as nesting), **except** a `{` after a `]` that closes a lambda introducer (defined below), which starts a lambda body; any other `{` or `}` at statement level starts or ends a block, which resets the statement. So statements inside a lambda body are checked like any others. |
| `STYLE-RAW-STRING` | as described under cleaning. |
| `ARCH-INCLUDE` | a file whose path starts with `src/agent/` has an `#include` line (spaces allowed around `#`; not inside a comment, judged on the cleaned text) whose path, written in `"..."` or `<...>`, has `experiment/` as a folder component after replacing `\` by `/`: at the start or after a `/`. This catches `experiment/x.h`, `../experiment/x.h`, `../../src/experiment/x.h`, `<experiment/x.h>` and `..\experiment\x.h`, but not `agent/experiment_notes.h` or `world/experiment.h` (a file, not a folder). |

**Lambda introducer.** A `[` token starts a lambda introducer when the token before it is none of these: an identifier (except the keyword `return`), a number, `)`, `]` or `[`. (So `a[3]`, `k[]`, `f()[0]` and the attribute `[[nodiscard]]` are not lambdas; `= [`, `( [`, `, [`, `return [` and a `[` at the start are.)

**Which braces are function bodies.** The checker keeps a stack of scopes. For each `{` it looks at the tokens since the previous `;`, `{` or `}` (the "head"), and applies the first matching case:
1. the head contains a lambda introducer `[...]` (its matching `]` found by counting brackets) followed either by nothing or by a parameter list `(...)`, and after that only tokens other than `(`, `)`, `[`, `]` and `=`, except that the words `noexcept` and `decltype` may each be followed by one balanced `(...)` group (for example `mutable`, `noexcept`, `noexcept(false)`, `-> std::pair<int, int>`, `-> decltype(x)`) → a **lambda body**, at any scope, including inside functions and inside other lambdas;
2. the head contains `namespace` → namespace scope;
3. the enclosing scope is a function, a lambda or a block → a block;
4. the head contains `class`, `struct`, `union` or `enum` and no `)` → class scope;
5. the head contains `)` → a **function body**, unless the head has an `=` outside parentheses and does not contain the word `operator` (then it is an initializer block);
6. otherwise → a block.

**Function bodies and lambda bodies are both subject to the 40-line limit** (`SIZE-FUNCTION`, same message). A lambda inside a function is measured on its own, and its lines also count towards the enclosing function (D-055).

**PowerShell files** (`run_checks.ps1`): `SIZE-FILE` as above. `SIZE-FUNCTION`: a line starting with `function <name>` begins a function, which ends at the line where its braces balance (counting `{` and `}`). The other rules do not apply to PowerShell.

**Which files are checked** (R1 scope), decided by **one** function, `checkedKind(relativePath)`, which returns `cpp`, `ps1` or nothing (not checked):
1. any path with a folder whose name starts with `cmake-build-` → not checked;
2. any path starting with `tools/reference/` → not checked (Claude's preserved Python reference);
3. a `.ps1` file at the repository root or anywhere under `tools/` → `ps1`;
4. a `.h` or `.cpp` file inside `src/`, `tests/` or `tools/` (not directly at the root) → `cpp`;
5. anything else (for example `docs/`, `.txt` files, a `.cpp` at the root) → not checked.

The tool walks the root folder (not recursively) and `src/`, `tests/` and `tools/` (recursively), skipping any folder named `.git`, and applies `checkedKind` to each file. Tests FS1–FS12 (6.2) fix the answers. The formatting step (3.5 step 8) gets its list of C++ files from the same function, through `code_checker --list-cpp <root>`, which prints the relative path of every `cpp` file, one per line, sorted, and nothing else (R4: one list of checked files, used twice).

**Output.** One line per violation, sorted by path, then line, then rule: `<path>:<line>: <RULE> <message>`. The messages are exactly:
- `file is <n> lines (limit 200)`
- `function is <n> lines (limit 40)`
- `'<keyword>' body must be in braces`
- `more than one statement on one line`
- `more than one declaration in one statement`
- `raw string literals are not allowed`
- `agent code must not include experiment code`

Then a summary line: `code checks: <files> files, <violations> violations`. The exit code is 0 only with no violations.

**Calibration (from the reference).** Run over the original repository's C++ code (`IWatchSheep/ai-hologram` at `89ea9b6`, 141 `.h` and `.cpp` files under `src/` and `tests/`; v1.0–v1.2 wrongly said 66), the rules find 125 violations, all genuine:
- 10 files over 200 lines;
- 28 functions over 40 lines;
- 58 one-line `if`/`for` bodies;
- 29 lines with several statements;
- no false positives found in a review of every hit.

This is what Cursor's squeezing and the old "about 200 lines" wording allowed. From now on, a run with even one violation fails. (The v1.1 rules give the same 125 on that code: it has no long lambdas and no agent folder.)

**Known limitations (tested and documented, D-056).** The checker reads text; it does not compile C++. These are the limits we know of; each tested one has an exact expected result.
| | Limitation | Test | What covers it instead |
|---|---|---|---|
| L1 | A comparison written without a space before `<` (`a<b`) is read as a template opening, so a second declaration after it can be missed. | CK46: `bool p = a<b, q = c>d;` gives **no** violation. | The project style (and `clang-format`, step 8) writes spaces around binary operators; review. |
| L2 | A comma expression (`a = 1, b = 2;`) is reported with the declaration message. | CK45 | Intended: comma expressions break "one statement per line" too. |
| L3 | The preprocessor is not run: code inside `#if 0` or produced by a macro is checked as written, and a macro that expands to braces is not seen. | — | Macros are not used for code structure in this project (review). |
| L4 | A lambda whose `[` directly follows an identifier other than `return`, a number, `)` or `]` is not recognised as a lambda (such code is unusual and unclear). | — | Review; any case found is added as a test by plan revision. |

A false positive or a missed violation found later is reported to Claude and settled by a plan revision with a new test. It is never worked around by squeezing or rearranging code.

### 3.5 `run_checks.ps1` (R14) — the full sequence
Run as `powershell -ExecutionPolicy Bypass -File .\run_checks.ps1` from the repository root. Each step prints one `[step] ...` line; the first failure prints `CHECKS FAILED: <reason>` in red and exits with code 1.
1. `[path]`: prepend CLion's MinGW `bin` folder and CLion's CMake `bin` folder (`...\bin\cmake\win\x64\bin`) to `PATH`, for this run only. If either folder is missing: fail, naming it.
2. `[provenance]`: as now (83 files; no carriage returns; SHA-256 matches).
3. `[configure]`: `cmake -S . -B cmake-build-checks -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Debug -DAI_HOLOGRAM_WARNINGS_AS_ERRORS=ON`. A separate folder, so CLion's own `cmake-build-debug` is never touched. Fails on a non-zero exit code. **Generator choice (O2):** "MinGW Makefiles" when `mingw32-make.exe` exists in CLion's MinGW `bin` folder; otherwise Ninja, when `C:\Program Files\JetBrains\CLion 2026.2.3.1\bin\ninja\win\x64\ninja.exe` exists (`-G Ninja -DCMAKE_MAKE_PROGRAM=<that path>`); otherwise fail, naming both paths. The step prints the generator it used. If the generator ever changes, CMake refuses to reuse the old folder; the owner then deletes `cmake-build-checks` (it holds only build output) and runs again.
4. `[build]`: `cmake --build cmake-build-checks`. Fails on a non-zero exit code (any warning is an error, 3.1), **and also** if the output contains `warning:` (second net); the compiler's messages are printed either way.
5. `[tests]`: `cmake-build-checks\ai_hologram_tests.exe` (quiet). Prints **all** its output lines. In quiet mode that is just the summary line when everything passes, and the `FAIL:` lines plus the summary when something fails, so the owner sees which test failed. Fails on a non-zero exit code.
6. `[ctest]`: `ctest --test-dir cmake-build-checks --output-on-failure`. Fails on a non-zero exit code.
7. `[code]`: `cmake-build-checks\code_checker.exe <root>`. Prints its violations (if any) and its summary line. Fails on a non-zero exit code.
8. `[format]`: the formatter is CLion's own copy at `C:\Program Files\JetBrains\CLion 2026.2.3.1\plugins\clion-radler\DotFiles\windows-x64\clang-format.exe` (found by the owner on 2026-10-20; there is none on `PATH`). The step prints `[format] clang-format <version>` from `--version`, then runs `clang-format --style=file --dry-run --Werror` on every file listed by `code_checker --list-cpp <root>` (3.4). It fails on any difference, printing clang-format's messages. **If that file is missing** (for example after a CLion update moves it), the run **fails**: `CHECKS FAILED: clang-format not found at <path>; formatting was not checked - ask Claude` (D-054). There is no partial pass.
9. `[program]`: run `cmake-build-checks\ai_hologram.exe`. Print `program output: <total> lines, <non-blank> non-blank`. Fails on a non-zero exit code.
10. `[git]`: `git status --short --branch`.
11. Final line, in green: `ALL CHECKS PASSED (<p> preserved files, <c> checks, <v> code files checked, formatting checked)`. It is printed **only** when every step 1–10 ran and passed; no step is ever skipped with a warning. So this line always means every mandatory check was done.
    - **During 0a only** (steps 7 and 8 are built in 0b), the final line is exactly `ALL CHECKS PASSED FOR STAGE 0a (83 preserved files, 5 checks; code and format checks are added in Stage 0b)`. The words say plainly which checks do not exist yet; from 0b on, only the full line above is allowed.

**Running programs from the script.** In Windows PowerShell 5.1, with `$ErrorActionPreference = "Stop"`, a program that writes to its error stream (as CMake and the compiler do for warnings) would stop the script with a confusing PowerShell error. So every external program (`cmake`, `ctest`, the built programs, `clang-format`, `git`) is run through **one** helper, `Invoke-Tool`, which sets `$ErrorActionPreference` to `Continue` for that call only, captures both streams as plain text lines (`2>&1`, each line converted to a string), restores the setting, and returns the lines and `$LASTEXITCODE`. Each step decides pass or fail from that exit code (R4: one helper for this job).

**The script is split by job (v1.3, D-057).** `run_checks.ps1` reached 198 of its 200 lines in 0a, with steps 7 and 8 still to come (the plan had estimated 180). It is therefore split into five files, each checked by the code checker (at most 200 lines, functions at most 40 lines):
| File | Holds |
|---|---|
| `run_checks.ps1` | The header, the settings (`$ErrorActionPreference`, `$root`, the build folder and the CLion paths), dot-sourcing the four files below, `Set-CompilerPath`, `Show-GitStatus`, the sequence of steps 1–10, and the final line. |
| `tools/checks/common.ps1` | `Stop-Checks`, `Invoke-Tool`, `Write-ToolLines` (the helpers every step uses). |
| `tools/checks/provenance.ps1` | `Test-OneFile`, `Test-Provenance` (step 2). |
| `tools/checks/build_steps.ps1` | `Invoke-Configure`, `Invoke-Build`, `Invoke-TestProgram`, `Invoke-Ctest`, `Invoke-Program` (steps 3–6 and 9). |
| `tools/checks/code_steps.ps1` | `Invoke-CodeChecker`, `Invoke-FormatCheck` (steps 7–8). |

The functions moved from 0a's `run_checks.ps1` keep their behaviour, except the `[tests]` change in step 5. They are moved, not rewritten. The four files contain only function definitions. `run_checks.ps1` dot-sources them (`. (Join-Path $root "tools/checks/common.ps1")`, and so on) before it runs any step. `Stop-Checks` still ends the whole run, because dot-sourced functions run in the script's own scope.


## 4. Files

| File | Approx. lines | Job | Sub-stage |
|---|---|---|---|
| `CMakeLists.txt` | 80 | The targets and CTest tests of 3.1 | 0a (checker targets added in 0b) |
| `src/core/project_info.h` / `.cpp` | 20 / 20 | `projectName()`, `currentStage()` | 0a |
| `src/main.cpp` | 20 | Prints the two lines of 3.2 | 0a |
| `tests/test_support.h` / `.cpp` | 30 / 30 | `TestCounter`, `check` (3.3) | 0a |
| `tests/test_suites.h` | 20 | Declarations of the test groups | 0a |
| `tests/test_main.cpp` | 30 | Runs the groups, prints the summary | 0a |
| `tests/test_project_info.cpp` | 30 | P1, P2, TS1–TS3 | 0a |
| `run_checks.ps1` | 198 in 0a → about 80 in 0b | 3.5; in 0b it keeps the sequence and dot-sources the files below | 0a, 0b |
| `tools/checks/common.ps1` | 45 | `Stop-Checks`, `Invoke-Tool`, `Write-ToolLines` (moved from `run_checks.ps1`) | 0b |
| `tools/checks/provenance.ps1` | 45 | Step 2 (moved) | 0b |
| `tools/checks/build_steps.ps1` | 110 | Steps 3–6 and 9 (moved; step 5 prints all lines) | 0b |
| `tools/checks/code_steps.ps1` | 70 | Steps 7–8 (new) | 0b |
| `tools/code_checker/text_cleaner.h` / `.cpp` | 30 / 120 | Cleaning and tokens | 0b |
| `tools/code_checker/size_rules.h` / `.cpp` | 20 / 70 | `SIZE-FILE`; `SIZE-FUNCTION` for PowerShell | 0b |
| `tools/code_checker/function_rule.h` / `.cpp` | 20 / 170 | Scope stack, lambda introducers, `SIZE-FUNCTION` for C++ functions and lambdas | 0b |
| `tools/code_checker/style_rules.h` / `.cpp` | 30 / 190 | `STYLE-BRACES`, `STYLE-STATEMENTS`, `STYLE-RAW-STRING` | 0b |
| `tools/code_checker/declaration_rule.h` / `.cpp` | 20 / 170 | `STYLE-DECLARATIONS` (including the lambda-body exception) | 0b |
| `tools/code_checker/architecture_rule.h` / `.cpp` | 20 / 60 | `ARCH-INCLUDE` (include paths, relative paths, backslashes) | 0b |
| `tools/code_checker/checker.h` / `.cpp` | 30 / 120 | `checkText(path, text, kind)` combining the rules; `checkedKind(path)`; sorting and formatting | 0b |
| `tools/code_checker/checker_main.cpp` | 110 | Walks the folders (3.4), reads files (CRLF → LF), prints, exit code; `--list-cpp` | 0b |
| `tests/test_code_checker_size.cpp` | 180 | CK1–CK5, CK20, CK22, CK25, CK26, CK29–CK33, CK35–CK38, CK50–CK52 | 0b |
| `tests/test_code_checker_style.cpp` | 120 | CK6–CK13, CK17, CK23, CK24, CK28 | 0b |
| `tests/test_code_checker_declarations.cpp` | 140 | CK14–CK16, CK21, CK27, CK34, CK39–CK46 | 0b |
| `tests/test_code_checker_architecture.cpp` | 110 | CK18, CK19, CK47–CK49 and FS1–FS12 (which files are checked) | 0b |
| `tools/reference/stage0/` | — | Claude's reference (committed with this plan) | plan |

**One shared helper per job (R4):** text cleaning and tokens live only in `text_cleaner`; the matching-bracket search only in `text_cleaner` (`findMatching`); the lambda-introducer test only in `function_rule` (`isLambdaIntroducer`), used by the declaration rule too. Long snippet texts in the tests are built by one helper (`repeatLine(text, count)`) in `tests/test_support`.

## 5. Expected output

**The program** (3.2): exactly the two lines shown.

**The tests:** `69 checks, 0 failures`, made up of:
- P1, P2: 2;
- TS1–TS3: 3;
- CK1–CK52: 52;
- FS1–FS12: 12.

**CTest:** 2 tests, both passing.

**The code checker on the repository:** `code checks: <n> files, 0 violations`. Here *n* is the number of checked files at the end of 0b: 27 C++ files (Section 4) plus 5 PowerShell files, so 32. Cursor reports the exact count. If a split approved by plan revision adds a file, the count changes; that is not an error.

**`run_checks.ps1`:** ends with `ALL CHECKS PASSED (83 preserved files, 69 checks, 32 code files checked, formatting checked)`, with the file count as just described.

## 6. Tests

### 6.1 Project info and the test runner (`tests/test_project_info.cpp`) — 0a
- **P1:** `projectName()` returns `"AI Hologram"`.
- **P2:** `currentStage()` returns 0.
- **TS1:** with a separate `TestCounter` whose `out` is a `std::ostringstream` and `verbose` false, a passing check writes nothing.
- **TS2:** the same counter, a failing check writes exactly `FAIL: x\n`, and that counter's `failures` is 1. The outer counter is unaffected.
- **TS3:** with `verbose` true, a passing check writes exactly `PASS: y\n`.

### 6.2 The code checker (`tests/test_code_checker*.cpp`) — 0b
Each CKn is **one** check: `checkText(path, text)` returns exactly the listed (line, rule) pairs, in order. Values come from the reference.

| Test | Path | Text (`\n` = new line) | Expected (line, rule) |
|---|---|---|---|
| CK1 | `src/a.cpp` | `int f() {\n    return 1;\n}\n` | none |
| CK2 | `src/a.cpp` | a function whose braces span 40 lines (`void f() {`, 38 lines `    g();`, `}`) | none |
| CK3 | `src/a.cpp` | the same with 41 lines | (1, SIZE-FUNCTION) |
| CK4 | `src/a.cpp` | 200 lines `int x;` | none |
| CK5 | `src/a.cpp` | 201 lines `int x;` | (1, SIZE-FILE) |
| CK6 | `src/a.cpp` | `void f() {\n    if (x) return;\n}\n` | (2, STYLE-BRACES) |
| CK7 | `src/a.cpp` | `void f() {\n    if (x) {\n        g();\n    } else g();\n}\n` | (4, STYLE-BRACES) |
| CK8 | `src/a.cpp` | `void f() {\n    if (x) {\n        g();\n    } else if (y) {\n        h();\n    }\n}\n` | none |
| CK9 | `src/a.cpp` | `void f() {\n    for (int i = 0; i < 3; ++i) g();\n    while (x) g();\n}\n` | (2, STYLE-BRACES), (3, STYLE-BRACES) |
| CK10 | `src/a.cpp` | `void f() {\n    do {\n        g();\n    } while (x);\n}\n` | none |
| CK11 | `src/a.cpp` | `void f() {\n    if (a &&\n        b) {\n        g();\n    }\n}\n` | none |
| CK12 | `src/a.cpp` | `void f() {\n    a = 1; b = 2;\n}\n` | (2, STYLE-STATEMENTS) |
| CK13 | `src/a.cpp` | `void f() {\n    std::string s = "a;b;c";\n    g(); // h(); k();\n}\n` | none |
| CK14 | `src/a.cpp` | `void f() {\n    int a, b;\n    int c = 1, d = 2;\n}\n` | (2, STYLE-DECLARATIONS), (3, STYLE-DECLARATIONS) |
| CK15 | `src/a.cpp` | `void f() {\n    std::map<int, int> m;\n    g(a, b);\n    std::vector<int> v{1, 2};\n    std::array<std::array<bool, 10>, 10> w{};\n    for (int i = 0, j = 0; i < 3; ++i) {\n        h();\n    }\n}\n` | none |
| CK16 | `src/a.cpp` | `enum class E { A, B };\nenum class F {\n    C,\n    D\n};\n` | none |
| CK17 | `src/a.cpp` | `void f() {\n    auto s = R"(x)";\n}\n` | (2, STYLE-RAW-STRING) |
| CK18 | `src/agent/a.cpp` | `#include "experiment/fault_event.h"\n#include "world/rules.h"\n` | (1, ARCH-INCLUDE) |
| CK19 | `src/experiment/a.cpp` | `#include "experiment/fault_event.h"\n` | none |
| CK20 | `src/a.cpp` | `struct S {` + `    bool operator==(const S& o) const {` + 40 lines `        g();` + `    }` + `};` | (2, SIZE-FUNCTION) |
| CK21 | `src/a.cpp` | `class C {\npublic:\n    C() : a_(1), b_(2) {\n    }\n\nprivate:\n    int a_;\n    int b_;\n};\n` | none |
| CK22 | `src/a.cpp` | `namespace n {`, `namespace {`, `const int k[] = {1, 2, 3};`, `}  // namespace`, then a 41-line function as CK3, then `}  // namespace n` | (5, SIZE-FUNCTION) |
| CK23 | `src/a.cpp` | `void f() {\n    auto g = [](int x) {\n        return x;\n    };\n    if (x) g(1);\n}\n` | (5, STYLE-BRACES) |
| CK24 | `src/a.cpp` | `/* if (x) return; a; b;\n   int a, b; */\nint f() {\n    return 0;\n}\n` | none |
| CK27 | `src/a.h` | `template <class A, class B>\nstruct P;\n` | none |
| CK25 | `run_checks.ps1` | `function A {`, 39 lines `    x`, `}` (41 lines) | (1, SIZE-FUNCTION) |
| CK26 | `run_checks.ps1` | the same with 38 lines `    x` (40 lines) | none |
| CK28 | `src/a.cpp` | as CK6 | the formatted line is exactly `src/a.cpp:2: STYLE-BRACES 'if' body must be in braces` |
| CK29 | `src/a.cpp` | namespace-level lambda: `auto g = [](int x) {`, 39 lines `    h();`, `};` (41 lines) | (1, SIZE-FUNCTION) |
| CK30 | `src/a.cpp` | the same with 38 lines `    h();` (40 lines) | none |
| CK31 | `src/a.cpp` | lambda inside a function: `void f() {`, `    auto g = [&](int x) {`, 39 lines `        h();`, `    };`, `}` | (1, SIZE-FUNCTION), (2, SIZE-FUNCTION) |
| CK32 | `src/a.cpp` | nested lambdas: `auto outer = []() {`, `    auto inner = [](int y) {`, 39 lines `        h();`, `    };`, `};` | (1, SIZE-FUNCTION), (2, SIZE-FUNCTION) |
| CK33 | `src/a.cpp` | lambda as an argument at namespace level: `const bool k = std::any_of(a, b, [](int x) {`, 39 lines `    h();`, `});` | (1, SIZE-FUNCTION) |
| CK34 | `src/a.cpp` | `void f() {\n    auto g = [a, b] {\n        int p, q;\n    };\n}\n` (captures, no parameter list) | (3, STYLE-DECLARATIONS) |
| CK35 | `src/a.cpp` | trailing return type: `auto f() -> std::pair<int, int> {`, 39 lines `    h();`, `}` | (1, SIZE-FUNCTION) |
| CK36 | `src/a.cpp` | member defined outside its class: `int S::get(const std::map<int, int>& m) const noexcept {`, 39 lines `    h();`, `}` | (1, SIZE-FUNCTION) |
| CK37 | `src/a.cpp` | attribute and a signature over two lines: `[[nodiscard]] int f(int a,`, `                  int b) {`, 39 lines `    h();`, `}` | (2, SIZE-FUNCTION) |
| CK38 | `src/a.cpp` | long initializers are not functions: `const int k[] = {`, 45 lines `    1,`, `};`, `const std::vector<int> v{`, 45 lines `    2,`, `};` | none |
| CK39 | `src/a.cpp` | `void f() {\n    bool c = a < b;\n    bool d = a > b;\n    bool e = a <= b, g = a >= b;\n}\n` | (4, STYLE-DECLARATIONS) |
| CK40 | `src/a.cpp` | `void f() {\n    int x = a << 2;\n    int y = a >> 1, z = 2;\n    std::cout << a << b;\n}\n` | (3, STYLE-DECLARATIONS) |
| CK41 | `src/a.cpp` | `void f() {\n    std::vector<std::vector<int>> v;\n    std::map<int, std::pair<int, int>> m, n;\n}\n` | (3, STYLE-DECLARATIONS) |
| CK42 | `src/a.cpp` | `void f() {\n    std::map<int, int> m{{1, 2}, {3, 4}};\n    int a[2][2] = {{1, 2}, {3, 4}};\n    S s = {1, 2}, t = {3, 4};\n}\n` | (4, STYLE-DECLARATIONS) |
| CK43 | `src/a.cpp` | `void f() {\n    int x = g(a, b);\n    h(a, g(b, c));\n    int y = g(a, b), z = 1;\n}\n` | (4, STYLE-DECLARATIONS) |
| CK44 | `src/a.cpp` | `void f() {\n    bool ok = x < y && y > z;\n    bool p = (x < y), q = (y > z);\n}\n` | (3, STYLE-DECLARATIONS) |
| CK45 | `src/a.cpp` | `void f() {\n    a = 1, b = 2;\n}\n` (limitation L2) | (2, STYLE-DECLARATIONS) |
| CK46 | `src/a.cpp` | `void f() {\n    bool p = a<b, q = c>d;\n}\n` (**limitation L1**: a missed violation, recorded as such) | none |
| CK47 | `src/agent/a.cpp` | `#include "../experiment/fault_event.h"\n#include "../../src/experiment/x.h"\n#include <experiment/y.h>\n#include "agent/experiment_notes.h"\n#include "world/experiment.h"\n` | (1, ARCH-INCLUDE), (2, ARCH-INCLUDE), (3, ARCH-INCLUDE) |
| CK48 | `src/agent/sub/b.h` | `// #include "experiment/x.h"\n/*\n#include "experiment/x.h"\n*/\n#include "..\..\experiment\x.h"\n` (the last line uses backslashes) | (5, ARCH-INCLUDE) |
| CK49 | `src/agent_tools/a.cpp` | `#include "experiment/x.h"\n` (not the agent folder) | none |
| CK50 | `src/a.cpp` | a returned lambda: `auto make() {`, `    return [](int x) {`, 39 lines `        h();`, `    };`, `}`, then `auto g() {\n    return [a, b] {\n        int p, q;\n    };\n}\n` | (1, SIZE-FUNCTION), (2, SIZE-FUNCTION), (46, STYLE-DECLARATIONS) |
| CK51 | `src/a.cpp` | conditional `noexcept`: `auto f = []() noexcept(false) {`, 39 lines `    h();`, `};` | (1, SIZE-FUNCTION) |
| CK52 | `src/a.cpp` | `auto g = [](int x) mutable noexcept(true) -> decltype(x) {`, 39 lines `    h();`, `};`, then `auto k = [](int x) noexcept(false) {\n    int p, q;\n};\n` | (1, SIZE-FUNCTION), (43, STYLE-DECLARATIONS) |

(CK25 and CK26 are checked with the PowerShell rules. The exact texts are in `tools/reference/stage0/snippets.py`; where this table describes a text in words, that file is the authority. Lines such as "39 lines `    h();`" are built with `repeatLine`.)

**Which files are checked (FS1–FS12), one check each:** `checkedKind(path)` returns exactly the value shown (`—` means not checked).

| Test | Path | Expected |
|---|---|---|
| FS1 | `src/core/a.cpp` | `cpp` |
| FS2 | `tests/test_main.cpp` | `cpp` |
| FS3 | `tools/code_checker/checker.h` | `cpp` |
| FS4 | `tools/reference/stage0/x.cpp` | — |
| FS5 | `cmake-build-checks/CMakeFiles/a.cpp` | — |
| FS6 | `src/cmake-build-debug/a.cpp` | — |
| FS7 | `docs/a.cpp` | — |
| FS8 | `run_checks.ps1` | `ps1` |
| FS9 | `tools/checks/build_steps.ps1` | `ps1` |
| FS10 | `tools/reference/stage4/gen.ps1` | — |
| FS11 | `src/notes.txt` | — |
| FS12 | `main.cpp` | — |

**What v1.0 missed (found by ChatGPT's review, confirmed on the v1.0 reference).** The v1.0 rules gave no violation for CK29, CK32, CK33, CK34 and CK47, and for CK48 they reported the commented-out include (line 3) instead of the real one (line 5). The v1.1 rules gave no violation for CK51 and missed the size violation in CK52 (ChatGPT's v1.1 review). The v1.2 rules give the results above.

## 7. Implementation order
| Step | Sub-stage | Content | Tests | Deliberate failure check (tried by Claude first, R11) |
|---|---|---|---|---|
| 1 | — | The owner commits this approved plan, the reference (`tools/reference/stage0/`) and the 0a prompt | | |
| 2 | **0a** | CMake (core library, program, tests), project info, test runner, `run_checks.ps1` steps 1–6 and 9–11 | P1, P2, TS1–TS3 | DF0a: edit one character in `docs/EXPERIMENT_1_PROTOCOL.md`. `run_checks.ps1` must stop at `[provenance]` naming that file; the owner restores it with `git checkout -- docs/EXPERIMENT_1_PROTOCOL.md`. |
| 3 | **0b** | The code checker (library, tool, CTest `code_checks`, `--list-cpp`), its tests, the split of `run_checks.ps1` into five files (3.5), steps 7–8, and the formatting of the 0a files (`src/main.cpp` and `tests/test_project_info.cpp` differ from `.clang-format` in two places: line wrapping only) | CK1–CK52, FS1–FS12 | DF0b-1: change the function-length limit from 40 to 41 (the C++ and the PowerShell rule use the one constant). Exactly CK3, CK22, CK25, CK29, CK31, CK32, CK33, CK35, CK36, CK37, CK50, CK51 and CK52 must fail. DF0b-2: make the braces rule ignore `else`. Exactly CK7 must fail. DF0b-3: make `isLambdaIntroducer` always return false. Exactly CK29, CK31, CK32, CK33, CK50, CK51 and CK52 must fail. DF0b-6: make the lambda-head rule reject every `(` after the parameter list (no `noexcept(...)` or `decltype(...)` groups). Exactly CK51 and CK52 must fail. DF0b-7: make `checkedKind` stop exempting `tools/reference/`. Exactly FS4 and FS10 must fail. (DF0b-1, -2, -3, -6 and -7 were tried on the reference: `df_check.py`.) DF0b-4: add `int a, b;` inside `main()` in `src/main.cpp`. The CTest test `code_checks` fails, so `run_checks.ps1` stops at `[ctest]` (step 6 runs before step 7) with a non-zero exit code; running `cmake-build-checks\code_checker.exe .` directly reports `src/main.cpp:<line>: STYLE-DECLARATIONS more than one declaration in one statement`. After the line is removed, the full run passes. (v1.0–v1.3 wrongly said `[code]`; corrected after ChatGPT's v1.3 review. The order of the steps is unchanged.) DF0b-5: add an unused local variable `int unused = 0;` inside `main()`. `run_checks.ps1` must stop at `[build]`, showing the compiler's `-Werror=unused-variable` message. DF0b-8: in `src/main.cpp`, join the two lines of the second `std::cout` statement into one line longer than 80 columns. `run_checks.ps1` must stop at `[format]`, naming `src/main.cpp`. |
| 4 | — | Fresh clone check: the owner clones the repository into a new folder, and `run_checks.ps1` passes there | | |
| 5 | — | Completion gate (Section 10), review by ChatGPT, tag `stage-0-v2` | | |

Work goes on a branch `stage-0-work` (R17). The owner commits and pushes it, Claude reviews it on GitHub, and the owner merges it into `main` after approval.

## 8. Decisions to record (with the plan's commit, R12)
| ID | Decision | Reason |
|---|---|---|
| D-050 | The code checker's exact rules (3.4), applied to `src/`, `tests/`, `tools/` C++ and `run_checks.ps1`; preserved and generated files are exempt. | R1, R2 and R7 enforced by code. `.clang-format` alone does not enforce braces or one statement per line (ChatGPT). |
| D-051 | Warnings are errors in the compiler itself (`-Werror` / `/WX`, option `AI_HOLOGRAM_WARNINGS_AS_ERRORS`, default `ON`); `run_checks.ps1` also fails on `warning:` in the output. It builds in its own folder `cmake-build-checks` with "MinGW Makefiles", or CLion's Ninja if that is missing. | ChatGPT: do not rely only on searching text. The owner's CLion build folder is never disturbed. |
| D-052 | Quiet test output by default; `--verbose` shows every PASS (R15). | The old output listed hundreds of PASS lines, which hid the result. |
| D-053 | Function length is counted from the `{` line to the `}` line, inclusive; file length excludes a final newline. | An exact, testable definition of "40 lines" and "200 lines". |
| D-054 | The formatting check uses CLion's bundled `clang-format.exe` (plugin folder) and prints its version. If it is missing, `run_checks.ps1` **fails**; `ALL CHECKS PASSED` is printed only when every mandatory step ran. | ChatGPT: the owner must never have to guess whether a mandatory check was done. It is the only copy on the owner's computer. |
| D-055 | Lambda bodies are functions for the 40-line limit, at every scope (namespace, function, nested lambda), as defined in 3.4, including heads with `noexcept(...)` and `decltype(...)`. | ChatGPT: v1.0 missed namespace-level and argument lambdas; v1.1 missed `noexcept(false)`. |
| D-056 | The code checker is a deterministic text heuristic. Its known limitations L1–L4 are listed in 3.4; L1 and L2 have tests with exact results (CK46, CK45). | ChatGPT (O3): accepted provided the limitations are tested and documented. |
| D-057 | `run_checks.ps1` is split by job into itself plus four dot-sourced files in `tools/checks/` (3.5). The moved functions keep their behaviour, except that `[tests]` now prints all of the quiet run's lines. | It reached 198 of 200 lines in 0a, before steps 7–8. The plan's estimate (180) was wrong. Splitting by job is the honest fix; squeezing is forbidden. Showing the `FAIL:` lines tells the owner which test failed. |
| D-058 | One function, `checkedKind`, decides which files are checked and how. The formatting step takes its list from `code_checker --list-cpp`. | One list of checked files, used twice (R4). PowerShell files under `tools/` are now checked too. |

## 9. Documentation
- **Headers.** Every new file has the three-part header (Purpose / "Introduced in Stage 0 (rebuild)" / Architecture).
- **README.** The status line becomes "Stage 0 of the rebuild: project skeleton, test program, code checker and run script" (0a, then 0b).
- **Handoff notes.** One per sub-stage, in `notes/handoff_notes.md`.

## 10. Completion checklist (owner verification gate)
- [ ] The plan was approved and committed before any code.
- [ ] `run_checks.ps1` passes on the owner's computer, **and on a fresh clone** (step 4).
- [ ] Program output exactly as in 3.2; tests `69 checks, 0 failures`; CTest 2/2.
- [ ] The code checker finds 0 violations in the repository, including in its own code and in `run_checks.ps1`.
- [ ] The deliberate failure checks gave exactly the predicted failures.
- [ ] No warnings in the build (enforced by step 4).
- [ ] D-050 to D-058 recorded; handoff notes appended; prompts saved in `docs/prompts/`.
- [ ] The owner can explain: what each `run_checks.ps1` step does; why the checker exists; what a deliberate failure check proves.

## 11. Things the owner should understand by the end
1. Why every rule is now checked by a program, not a promise.
2. What each step of `run_checks.ps1` checks, and why it stops at the first failure.
3. What the code checker looks for, and why it ignores comments and text inside quotes.
4. Why a fresh clone must pass too.

## 12. What actually happened (filled in at the end)
```
Date completed:
Commit tagged stage-0-v2:
Checks:
Did everything go as planned?
```

## 13. Instructions to the coding assistant
Every sub-stage prompt says: follow `AGENTS.md`; implement only that sub-stage; never commit; never change an expected value; run `run_checks.ps1` and report its final line exactly.

## 14. Open points for review
- **O1. Settled (2026-10-20).** CLion 2026.2.3.1 has no `clang-format.exe` in `bin\clang\win\x64\bin`. It has one inside its C++ engine plugin, `plugins\clion-radler\DotFiles\windows-x64\clang-format.exe`, which step 8 uses. On 2026-10-20 it reports `clang-format version 22.0.0git` (a JetBrains build of LLVM 22), new enough for every option in `.clang-format`, including `InsertBraces` (version 15 or newer). A CLion update may move it or change its version: step 8 prints the version on every run, and a missing file fails the run (D-054). A different version is reported to Claude, not worked around.
- **O2. Settled by ChatGPT's review.** "MinGW Makefiles", with CLion's bundled Ninja as the documented fallback (3.5 step 3).
- **O3. Settled by ChatGPT's review.** A deterministic heuristic checker is accepted, with its known limitations tested and documented (3.4, D-056).

## Revision history
| Version | Date | Change |
|---|---|---|
| 1.0 | 2026-10-20 | First draft, with the reference built first (R9). O1 settled the same day: CLion's plugin copy of `clang-format` (D-054). |
| 1.1 | 2026-10-20 | ChatGPT's four corrections. (1) Lambda bodies count as functions at every scope (3.4, D-055); tests CK29–CK33, CK35–CK38, CK50. (2) Adversarial declaration tests CK34, CK39–CK46, and relative, angle-bracket and backslash include paths CK47–CK49; known limitations L1–L4 documented (D-056). (3) A missing `clang-format` fails the run; `ALL CHECKS PASSED` only when every step ran (D-054). (4) Warnings are compiler errors (`-Werror` / `/WX`), with the text search kept as a second net (D-051); DF0b-5 added. O2 and O3 settled. Tests 33 → 55. Files: `function_rule` added; the checker tests split into four named files. All new expected values come from the revised reference. One of Claude's hand predictions (CK50, line 47) was miscounted; the reference's line 46 is correct. |
| 1.2 | 2026-10-20 | ChatGPT approved v1.1 after re-running the reference (49 cases, DF0b-1/2/3 as predicted), with one clarification: a lambda with a conditional exception specification, `[]() noexcept(false) { ... }`, was not recognised. The lambda-head rule now allows one balanced `(...)` after `noexcept` and after `decltype` (3.4, D-055). New tests CK51, CK52 and deliberate failure DF0b-6. Tests 55 → 57. Again one of Claude's hand predictions was miscounted (CK52, line 44); the reference's line 43 is correct. Calibration unchanged (125). Also added before the owner's approval: the exact 0a final line, and the `Invoke-Tool` helper for running programs from PowerShell 5.1 (3.5). DF0a was tried by Claude on a copy of the repository: `CHECKS FAILED: docs/EXPERIMENT_1_PROTOCOL.md was changed (SHA-256 differs from docs/PROVENANCE.sha256)`, exit code 1; after `git checkout` the run passed again. |
| 1.3 | 2026-10-20 | Revision for Stage 0b, after 0a was completed (`f1f8086`; DF0a passed). (1) `run_checks.ps1` reached 198 of 200 lines in 0a, so it is split by job into five files (3.5, D-057). (2) `[tests]` prints all lines, so a failing test is named. (3) One `checkedKind` function decides which files are checked, PowerShell files under `tools/` included; the formatting step uses the same list via `--list-cpp` (3.4, D-058); tests FS1–FS12, deliberate failures DF0b-7 and DF0b-8. (4) Two formatting differences in 0a files are fixed in 0b. (5) Calibration file count corrected from 66 to 141 (the 125 violations were right). Tests 57 → 69; files checked 28 → 32. After ChatGPT's v1.3 review (reference re-run: all 51 snippets, FS1–FS12 and DF0b-1/2/3/6/7 as predicted; D-057 and D-058 accepted), DF0b-4's expected stopping point was corrected from `[code]` to `[ctest]`, because CTest's `code_checks` test runs first. |
