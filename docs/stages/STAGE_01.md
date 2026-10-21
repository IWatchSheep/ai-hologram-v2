# Stage 1 (rebuild) — The grid world

| | |
|---|---|
| **Project** | AI Hologram v2 (`ai-hologram-v2`) |
| **Stage** | 1 of the rebuild (Brief Section 10, Stage 1: the grid world) |
| **Plan version** | 1.2 (after Stage 1a: G1 gets its own file; the golden readers become shared, bounds-safe and tested with malformed files) |
| **Date** | October 20, 2026 |
| **Status** | **APPROVED.** v1.1 approved by ChatGPT and the owner (Stage 1a was built under it). v1.2, which changes only Stage 1b, approved by ChatGPT (with its malformed-file safeguard added as GN1–GN6 and GR1–GR5) and by the owner on 2026-10-20 ("I approve. thank you."). Stage 1b may begin after v1.2 is committed. |
| **Governing documents** | The brief v1.1; `docs/RULES.md` (R1–R24); Migration Addendum M1; `AGENTS.md`. |
| **Builds on** | Stage 0, complete and tagged `stage-0-v2` (`148ea6e`). |
| **Original it rebuilds** | `IWatchSheep/ai-hologram`, Stage 1 (plan `docs/history/ai-hologram/docs/stages/STAGE_01.md` v1.4, tag `stage-1`). Its world code, `src/world/`, never changed from `stage-1` to the last commit `89ea9b6`, so it is also the world of the frozen C1 baseline (`stage-3`, `7587f38`). |
| **Reference** | `tools/reference/stage1/` (R9, R10): `ref_world.py`, the check mirror `checks.py`, `df_check.py`, `gen_golden.py`, and the golden values produced by a tool built from the **original C++** (`orig_trace/`). |

---

## 1. Purpose
Stage 1 rebuilds the **world**, the ground truth that every later stage reasons about. It is a 10×10 grid with walls, one agent, one movable object and one destination, plus the rules for what happens when the agent tries to move. **It must behave exactly like the original world**, because the rebuilt C1 baseline will later have to reproduce the original's behaviour step for step (M1.4).

Stage 1 is the first stage that can be compared with the original code directly. So besides the usual hand-written tests, it gets **golden values**:
- 5,184 rule cases;
- 1,009 parser cases.

Both were produced once by a small tool compiled from the **original** world source code. The rebuilt world must reproduce every one of them.

When Stage 1 is complete, the owner can:
- describe a world as 10 lines of text and have the program read it;
- apply an action and see exactly what happened (moved, pushed, blocked);
- print the world as text;
- see that the rebuilt world agrees with the original on every golden case.

## 2. What Stage 1 must NOT contain
- No reasoning: no planning, search, beliefs, predictions or comparisons.
- The "agent" is only a position on the grid; nothing chooses actions except the hand-written demo list.
- No faults, budgets, messages, randomness or experiment code.
- No trap detection.
- No external libraries, exceptions, assertions that stop the program, or inheritance.

## 3. The world: exact rules
These are the original rules (original plan 3.1–3.6), unchanged. The code implements them exactly.

### 3.1 Grid and coordinates
- The grid is 10 columns × 10 rows.
- A position is **(x, y)**:
  - x is the column, 0–9, from left to right;
  - y is the row, 0–9, from top to bottom (row 0 is the first text line).
- The directions:

  | Direction | Meaning | Change |
  |---|---|---|
  | North | up | y − 1 |
  | South | down | y + 1 |
  | East | right | x + 1 |
  | West | left | x − 1 |

### 3.2 Contents
- **Walls:** any number. Every outer border cell is a wall; walls never move.
- **Agent:** exactly 1, on a non-wall cell.
- **Object:** exactly 1, on a non-wall cell, never on the agent's cell.
- **Destination:** exactly 1, on a non-wall cell. The agent and the object may stand on it.

### 3.3 Actions
Exactly four: **North, East, South, West**. There is no "wait" action. A safe stop, in later stages, means not calling the world at all.

### 3.4 Movement rules (in this order)
Let T be the cell next to the agent in the chosen direction, and B the cell beyond T.

0. **The state is not valid** (3.6) → nothing changes; the outcome is **InvalidState**.
1. **T is a wall** → nothing changes: **Blocked**.
2. **T is not the object's cell** → the agent moves to T: **Moved**.
3. **T holds the object, and B is a wall** → nothing changes: **Blocked**.
4. **T holds the object, and B is not a wall** → the object moves to B and the agent to T: **Pushed**.

Further rules:
- The agent can push, never pull.
- The destination never blocks anything.
- **The goal is reached** when the object is on the destination. Actions still apply normally afterwards.
- Every attempted action counts as one step, blocked ones included.

### 3.5 Text format
A world is exactly 10 lines of exactly 10 characters:

| Character | Meaning |
|---|---|
| `#` | wall |
| `.` | floor |
| `A` | agent |
| `O` | object |
| `D` | destination |
| `*` | object on the destination |
| `+` | agent on the destination |

**Parsing (`parseWorld`) checks in exactly this order** and returns the first error, with exactly these messages:
1. The number of lines is not 10: `A world must contain exactly 10 lines.`
2. Any line's length is not 10: `Every world line must contain exactly 10 characters.` All lengths are checked before any character is read.
3. Cells are read row by row (y = 0…9), and left to right within a row (x = 0…9). For each cell:
   - a. a border cell (x = 0, x = 9, y = 0 or y = 9) that is not `#`: `Every border cell must be a wall (#).`
   - b. otherwise, a character outside the table: `The world contains an unknown character.`

   `A` and `+` count one agent; `O` and `*` count one object; `D`, `*` and `+` count one destination. Each also records its position, and a later one overwrites an earlier one.
4. The agent count is not 1: `A world must contain exactly one agent (A or +).`
5. The object count is not 1: `A world must contain exactly one object (O or *).`
6. The destination count is not 1: `A world must contain exactly one destination (D, * or +).`
7. `isValidWorld` is false: `The parsed world state is not valid.` (This cannot happen after steps 1–6, but the check stays, as in the original.)

On success the error text is empty.

**Rendering (`renderWorld`)** gives 10 strings of 10 characters. For each cell, in this order: wall → `#`; agent and destination → `+`; object and destination → `*`; agent → `A`; object → `O`; destination → `D`; otherwise `.`. It requires a valid state (3.6). It indexes walls only with its own loop counters 0–9.

### 3.6 Valid states and safety
A `WorldState` is valid when:
1. the agent, object and destination are all inside the grid;
2. every border cell is a wall;
3. none of the three is on a wall;
4. the agent and the object are on different cells.

There is **one** function for this, `isValidWorld`. `parseWorld` uses it (step 7), and `applyAction` calls it first (rule 0).

`isWall(state, p)` returns true for any p outside the grid, and reads the walls array only after checking that p is inside. `Environment` holds the one true state: `step(action)` applies the rules, stores the result, and adds one to the step count, even when the outcome is InvalidState.

### 3.7 Golden values from the original code (new in the rebuild)
**How they were made** (all in `tools/reference/stage1/`):
1. `orig_trace/original_src/` holds the original `src/world/*` and `src/demo/scripted_demo.*` files from commit `7587f38`, byte for byte (13 files). Each SHA-256 was verified against `git show 7587f38:src/<file>`, and all 13 are in `docs/PROVENANCE.sha256`, so `run_checks.ps1` protects them like the golden files (v1.1).
2. `orig_trace/orig_world_trace.cpp` is a small tool compiled **only** against those original files.
3. `gen_golden.py cases` writes the case lists, deterministically: a fixed list, plus a fixed random seed for the parser mutations.
4. The original tool ran on the case lists. Its output is the golden files. `ref_world.py` produces **byte-identical** files, which is the reference contract (R10).

**`golden/world_rules_golden.txt`** (5,206 lines):
- **Layouts.** It first gives two layouts, `L1` (the demo layout) and `L2` (an internal maze). Each is the line `layout <id>` followed by its 10 lines.
- **Cases.** Then 5,184 lines, each of the form `<id> <ax> <ay> <ox> <oy> <Action> -> <Outcome> <ax'> <ay'> <ox'> <oy'> <goal>`.
  - Each case takes the layout's walls and destination, sets the agent to (ax, ay) and the object to (ox, oy), and applies the action.
  - The result is the outcome name, the new agent and object positions, and `1` or `0` for `isGoalReached`.
- **Coverage.** Agent positions are every cell of the grid, walls included, plus eight positions outside it. Object positions are the four neighbours of the agent, the agent's own cell, and (5, 5). All four actions are used.
- **Outcomes.** 924 Moved, 230 Pushed, 454 Blocked and 3,576 InvalidState.

**`golden/world_parse_golden.txt`** (1,009 cases):
- **Format.** Each case is the line `case <n> lines <k>`, followed by its k input lines exactly, and then one of:
  - `expect OK <ax> <ay> <ox> <oy> <dx> <dy>`, with ` ROUNDTRIP-DIFFERS` added if rendering the parsed world does not give back the input (this never occurs);
  - `expect ERROR <message>`.
- **Cases.** The fixed cases are an empty input, ten empty lines, 1,000-character lines, 9 lines, 11 lines and four valid layouts. Then come 1,000 mutations: random cells replaced with characters from `#.AOD*+x?`, and sometimes a line removed or added, or one line made a character shorter or longer.

**SHA-256** (also added to `docs/PROVENANCE.sha256`, so `run_checks.ps1` verifies them as preserved files):
```
f8e13c1d7f01e352ad5bd98e5459f0b5213d10422f096c026cdfb3a031d5c505  tools/reference/stage1/golden/world_rules_golden.txt
205b7c078370426a175275b72ee7686280f362dd2ca944d5f96476e11f6512c9  tools/reference/stage1/golden/world_parse_golden.txt
```

### 3.8 The program
`ai_hologram` prints two header lines, then the scripted demonstration. The demo starts with an empty line, so line 3 is blank. Exactly:
```
AI Hologram v2 -- Stage 1
Grid world: environment rules only (scripted demonstration). No reasoning components yet.

Initial world:
##########
#........#
#........#
#..A.O..D#
#........#
#........#
#...##...#
#........#
#........#
##########

Step 1: East -> Moved
Step 2: East -> Pushed
Step 3: East -> Pushed
Step 4: East -> Pushed
Step 5: East -> Blocked

Final world:
##########
#........#
#........#
#......A*#
#........#
#........#
#...##...#
#........#
#........#
##########
Goal reached: yes
Steps taken: 5
```
The output has 34 lines, 31 of them non-blank. The demo part, from the first empty line to the end, is byte-identical to the original's `runScriptedDemo` output: checked against the original tool. The header differs from the original on purpose (D-062).

## 4. Files
All new code is in namespace `aih`, compiled into `ai_hologram_core`.

| File | Approx. lines | Job | Sub-stage |
|---|---|---|---|
| `src/world/position.h` | 35 | `Position {x, y}`, `==`, `!=` | 1a |
| `src/world/action.h` / `.cpp` | 35 / 55 | `Action`, `actionName`, `offsetFor`, `moved` | 1a |
| `src/world/world_state.h` / `.cpp` | 55 / 85 | `gridSize`, `WorldState` (walls row-first `walls[y][x]`), `==`, `isInsideGrid`, `isWall`, `isValidWorld`, `isGoalReached` | 1a |
| `src/world/world_text.h` | 40 | `ParseResult {std::optional<WorldState> world; std::string error;}`, `parseWorld`, `renderWorld` | 1a |
| `src/world/world_parse.cpp` | 170 | `parseWorld` and its helpers (3.5) | 1a |
| `src/world/world_render.cpp` | 55 | `renderWorld` | 1a |
| `src/world/rules.h` / `.cpp` | 40 / 65 | `MoveOutcome`, `outcomeName`, `ActionResult`, `applyAction` (pure: takes `const WorldState&`, returns a new state) | 1b |
| `src/world/environment.h` / `.cpp` | 45 / 40 | `Environment`: private state, `step`, `state() const`, `stepCount() const` | 1b |
| `src/demo/scripted_demo.h` / `.cpp` | 25 / 90 | `runScriptedDemo(std::ostream&)` (3.8; the actions are hand-written) | 1c |
| `src/main.cpp` | 20 | Prints the two header lines, then calls the demo | 1c (changed) |
| `src/core/project_info.cpp` | 20 | `currentStage()` returns 1 | 1c (changed) |
| `tests/test_layouts.h` / `.cpp` | 30 / 70 | The hand-drawn test layouts of 6.1 and `withLine` (R4: one copy) | 1a |
| `tests/golden_files.h` / `.cpp` | 30 / 60 | `readLines(path)`: reads a text file into lines, removing any `\r` (R4); from 1b also `parseInteger(text, value)`, the one integer test | 1a, 1b |
| `tests/test_world_parse.cpp` | 170 | T1–T20 | 1a |
| `tests/test_world_render.cpp` | 50 | RT1–RT3 | 1a |
| `tests/test_world_state.cpp` | 110 | V1–V6, N1 | 1a |
| `tests/test_world_golden.cpp` | 130 | G2 (1a); in 1b its reader moves to `golden_parse_reader.cpp`, and GN1–GN6 are added | 1a, 1b |
| `tests/test_world_golden_rules.cpp` | 110 | G1 and GR1–GR5 (v1.2: its own file) | 1b |
| `tests/golden_readers.h` | 45 | `GoldenCheck {bool ok; int cases; std::string problem;}`, `checkParseGoldenLines`, `checkRulesGoldenLines` (v1.2) | 1b |
| `tests/golden_parse_reader.cpp` | 110 | The parse-golden reader (6.7), moved from `test_world_golden.cpp` and made bounds-safe | 1b |
| `tests/golden_rules_reader.cpp` | 150 | The rules-golden reader (6.7) | 1b |
| `tests/test_rules.cpp` | 190 | R1–R11, N2 | 1b |
| `tests/test_environment.cpp` | 90 | E1–E4 | 1b |
| `tests/test_demo.cpp` | 60 | D1 | 1c |
| `tests/test_suites.h`, `tests/test_main.cpp`, `tests/test_project_info.cpp` | — | New groups; P2 checks 1 from 1c | 1a–1c (changed) |
| `CMakeLists.txt` | — | New sources and tests; the compile definition `AIH_SOURCE_DIR="${CMAKE_SOURCE_DIR}"` for `ai_hologram_tests`, so the golden tests find `tools/reference/stage1/golden/` | 1a (changed) |
| `tools/reference/stage1/` | — | Claude's reference and the golden values (committed with this plan) | plan |

**Splits planned in advance** (lessons from Stage 0):
- `world_text.cpp` (172 lines in the original) is split into `world_parse.cpp` and `world_render.cpp`. They are two jobs: reading and writing.
- The original 237-line `test_world_text.cpp` becomes `test_world_parse.cpp`, `test_world_render.cpp` and `test_world_state.cpp`.
- `test_rules.cpp` is expected to be the fullest file. If either cannot hold its tests cleanly within 200 lines, the coding assistant stops and reports; nothing is squeezed.

**One shared helper per job (R4):**
- the layouts and `withLine` only in `test_layouts`;
- file reading only in `golden_files`;
- validity only in `isValidWorld`;
- outside-the-grid handling only in `isWall`;
- the action offset only in `offsetFor`.

## 5. Expected output
- **The program:** exactly 3.8.
- **The tests:**

  | After | Expected line |
  |---|---|
  | 1a | `105 checks, 0 failures` |
  | 1b | `139 checks, 0 failures` |
  | 1c | `140 checks, 0 failures` |

  The 140 are:
  - the 71 Stage 0 checks (P2 now checks 1);
  - T1–T20: 20;
  - RT1–RT3: 3;
  - V1–V5 and V6a–d: 9;
  - N1, N2: 2;
  - R1a–d, R2–R7, R8a–c, R9, R10, R11a–b: 17;
  - E1–E4: 4;
  - G1, G2: 2;
  - GN1–GN6, GR1–GR5: 11 (v1.2);
  - D1: 1.
- **CTest:** 2/2.
- **Code checker:** 0 violations.

  | After | Files checked |
  |---|---|
  | 1a | 48 |
  | 1b | 58 |
  | 1c | 61 |

  These are the Stage 0 files plus the new files of Section 4, at the stated sizes. If a split approved by plan revision adds a file, the count changes.
- **`run_checks.ps1`** ends with `ALL CHECKS PASSED (98 preserved files, 140 checks, 61 code files checked, formatting checked)`. The two golden files and the 13 original source files bring the preserved files from 83 to 98.

## 6. Tests
Every check's description starts with its id. `checks.py` mirrors each check on the reference: `failing: none (69 checks)`. In it, N1 and N2 are placeholders, because Python has no enum names.

### 6.1 Test layouts (`tests/test_layouts`), exactly
```
DEMO          STAR          PLUS          OPEN
##########    ##########    ##########    ##########
#........#    #........#    #........#    #........#
#........#    #........#    #........#    #........#
#..A.O..D#    #..A....*#    #..O....+#    #........#
#........#    #........#    #........#    #...A....#
#........#    #........#    #........#    #........#
#...##...#    #...##...#    #...##...#    #........#
#........#    #........#    #........#    #......O.#
#........#    #........#    #........#    #D.......#
##########    ##########    ##########    ##########
```
- `withLine(layout, index, text)` returns a copy with one line replaced.
- `at(layout, agent, object)` returns the parsed layout with the agent and object moved, written directly into the state.

### 6.2 Parsing (`test_world_parse.cpp`), 1a
| Id | Check |
|---|---|
| T1 | DEMO parses: the world is present, and the error is empty |
| T2 | DEMO: agent (3,3), object (5,3), destination (8,3) |
| T3 | DEMO: walls at (0,0), (4,6), (5,6); floor at (1,1), (4,5) |
| T4 | DEMO's first 9 lines → `A world must contain exactly 10 lines.` |
| T5 | DEMO plus an 11th line `##########` → the same message |
| T6 | DEMO with line 4 = `#.......#` (9 characters) → `Every world line must contain exactly 10 characters.` |
| T7 | DEMO with line 4 = `#.........#` (11 characters) → the same message |
| T8 | DEMO with line 9 = `####.#####` → `Every border cell must be a wall (#).` |
| T9 | DEMO with line 3 = `#....O..D#` (no agent) → `A world must contain exactly one agent (A or +).` |
| T10 | DEMO with line 1 = `#A.......#` (two agents) → the agent message |
| T11 | DEMO with line 3 = `#..A....D#` (no object) → `A world must contain exactly one object (O or *).` |
| T12 | DEMO with line 1 = `#O.......#` (two objects) → the object message |
| T13 | DEMO with line 3 = `#..A.O...#` (no destination) → `A world must contain exactly one destination (D, * or +).` |
| T14 | DEMO with line 1 = `#D.......#` (two destinations) → the destination message |
| T15 | DEMO with line 2 = `#...x....#` → `The world contains an unknown character.` |
| T16 | STAR: object (8,3), destination (8,3), agent (3,3) |
| T17 | PLUS: agent (8,3), destination (8,3), object (3,3) |
| T18 | an empty list of lines → the lines message |
| T19 | ten empty strings → the length message |
| T20 | ten lines of 1,000 `#` → the length message |

### 6.3 Rendering (`test_world_render.cpp`), 1a
- **RT1–RT3:** `renderWorld(parseWorld(X))` equals X exactly, for X = DEMO, STAR and PLUS.

### 6.4 Validity and names (`test_world_state.cpp`), 1a
| Id | Check |
|---|---|
| V1 | parsed DEMO is valid |
| V2 | DEMO with the agent set to (4,6), a wall → invalid |
| V3 | DEMO with the agent set to the object's cell → invalid |
| V4 | DEMO with `walls[0][5]` set to false → invalid |
| V5 | DEMO with the agent at (10,3) is invalid, **and** DEMO with the object at (3,−1) is invalid (one check) |
| V6a–d | `isWall(DEMO, p)` is true for p = (−1,0), (10,0), (0,−1), (0,10): four checks |
| N1 | `actionName` gives `North`, `East`, `South`, `West` for the four actions |

### 6.5 Rules (`test_rules.cpp`), 1b
All use OPEN; `at(...)` sets the agent and object.

| Id | Check |
|---|---|
| R1a–d | From OPEN, each of North, East, South, West: the agent moves to (4,3), (5,4), (4,5), (3,4) respectively; the outcome is Moved; the object stays at (7,7). Four checks. |
| R2 | `at(OPEN,(1,1),(7,7))` North → Blocked, and the whole state is unchanged |
| R3 | `at(OPEN,(4,4),(5,4))` East → Pushed; agent (5,4), object (6,4) |
| R4 | `at(OPEN,(7,4),(8,4))` East → Blocked; agent (7,4), object (8,4) |
| R5 | `at(OPEN,(3,8),(2,8))` West → Pushed, and the goal is reached |
| R6 | `at(OPEN,(2,7),(5,5))`: West, then South → the second outcome is Moved; the agent is at (1,8), on the destination; the goal is **not** reached |
| R7 | `at(OPEN,(4,4),(5,4))` West → Moved, and the object stays at (5,4) (no pulling) |
| R8a–c | The walls are unchanged after a move (OPEN North), a push (as R3) and a blocked push (as R4). Three checks. |
| R9 | `applyAction` does not change its input: the R3 state is identical before and after |
| R10 | `at(OPEN,(2,7),(1,8))` has the goal reached; East → Moved, agent (3,7), object stays (1,8) |
| R11a | `at(OPEN,(10,4),(7,7))` West → InvalidState, and the state is unchanged |
| R11b | `at(OPEN,(0,4),(7,7))` East → InvalidState, and the state is unchanged |
| N2 | `outcomeName` gives `Moved`, `Pushed`, `Blocked`, `InvalidState` |

### 6.6 Environment (`test_environment.cpp`), 1b
| Id | Check |
|---|---|
| E1 | a new Environment from OPEN: its state equals OPEN, and the step count is 0 |
| E2 | North four times: the step count is 4 (the fourth is blocked), and the agent is at (4,1) |
| E3 | East, East, South, South, South, East, West, North: the Environment's state equals `applyAction` applied by hand in the same order, and the step count is 8 |
| E4 | an Environment from `at(OPEN,(0,4),(7,7))`: `step(East)` gives InvalidState, the state is unchanged, and the step count is 1 |

### 6.7 Golden values (`test_world_golden.cpp`, `test_world_golden_rules.cpp`)
- **G2 (1a):** reads `world_parse_golden.txt` with `readLines`.
  - For each case it calls `parseWorld` on the k lines, and formats the result exactly as the `expect` line: OK with its six numbers and the round-trip suffix, or ERROR with the message.
  - It passes only if every case matches **and** there are exactly 1,009 cases.
  - On the first mismatch it prints the case number, the expected text and the actual text.
- **G1 (1b):** reads `world_rules_golden.txt`.
  - It parses each layout with `parseWorld`, then for each case line rebuilds the left-hand side, applies the action, formats the right-hand side, and compares the whole line.
  - It passes only if every line matches **and** there are exactly 5,184 cases.
  - On the first mismatch it prints the line number, the expected text and the actual text.

A missing or unreadable golden file is a failure with a clear message, never a pass.

**The readers (v1.2).** Both live in `tests/golden_*_reader.cpp`, behind `tests/golden_readers.h`. Each takes the file's lines and returns `GoldenCheck {ok, cases, problem}`; `problem` is empty when ok. G1 and G2 call them on the real files and pass only if `ok` is true and `cases` is 5,184 or 1,009. They follow `tools/reference/stage1/ref_golden_readers.py` exactly. Line numbers count from 1. Before every access, the reader checks that the line exists. It never reads out of range and never throws (the project uses no exceptions).

- **Parse reader**, per case:
  - the header must be exactly four space-separated words `case <n> lines <k>`, with `<n>` and `<k>` integers (`parseInteger`: an optional `-`, then digits only) and k ≥ 0; otherwise `line <i>: invalid case header`;
  - fewer than k lines remaining: `case <n>: missing input lines`;
  - no line after the k inputs: `case <n>: missing expect line`;
  - that line not starting with `expect `: `case <n>: malformed expect line`;
  - a different result: `case <n>: expected [<expected>] actual [<actual>]`.
- **Rules reader**, per line:
  - `layout <id>` with fewer than 10 lines after it: `line <i>: truncated layout <id>`;
  - a layout that does not parse: `line <i>: layout <id> does not parse: <error>`;
  - a line without ` -> `, or whose left side is not exactly six words `<id> <ax> <ay> <ox> <oy> <Action>` (four integers and one of the four action names): `line <i>: malformed case line`;
  - an id with no layout above it: `line <i>: unknown layout <id>`;
  - a different line: `line <i>: expected [<line>] actual [<line>]`.

**Malformed-file tests (v1.2, ChatGPT's safeguard).** Each is one check comparing the whole `GoldenCheck` (ok, cases, problem) exactly. DEMO is the layout of 6.1, and E is `expect ERROR A world must contain exactly 10 lines.`

| Id | Reader | Lines | Expected (ok, cases, problem) |
|---|---|---|---|
| GN1 | parse | `case 1 lines 10`, then DEMO's first 3 lines | (false, 0, `case 1: missing input lines`) |
| GN2 | parse | `case 1 lines 2`, `a`, `b` | (false, 0, `case 1: missing expect line`) |
| GN3 | parse | `case 1 lines 0`, `result OK` | (false, 0, `case 1: malformed expect line`) |
| GN4 | parse | `case 1 lines -1`, E | (false, 0, `line 1: invalid case header`) |
| GN5 | parse | `case 1 lines x`, E | (false, 0, `line 1: invalid case header`) |
| GN6 | parse | `case 1 lines 0`, E | (true, 1, empty), a positive control |
| GR1 | rules | `layout L1`, then DEMO's first 4 lines | (false, 0, `line 1: truncated layout L1`) |
| GR2 | rules | `layout L1`, DEMO, `L1 3 3 5 3` | (false, 0, `line 12: malformed case line`) |
| GR3 | rules | `layout L1`, DEMO, `L9 3 3 5 3 East -> Moved 4 3 5 3 0` | (false, 0, `line 12: unknown layout L9`) |
| GR4 | rules | `layout L1`, DEMO, `L1 3 3 5 3 East -> Moved 4 3 5 3` | (false, 1, `line 12: expected [L1 3 3 5 3 East -> Moved 4 3 5 3] actual [L1 3 3 5 3 East -> Moved 4 3 5 3 0]`) |
| GR5 | rules | `layout L1`, DEMO, `L1 3 3 5 3 East -> Moved 4 3 5 3 0` | (true, 1, empty), a positive control |

### 6.8 Demo (`test_demo.cpp`), 1c
- **D1:** `runScriptedDemo` writing to a `std::ostringstream` produces exactly the demo part of 3.8, from the first empty line to `Steps taken: 5\n`.

## 7. Implementation order
| Step | Sub-stage | Content | Tests | Deliberate failure check (tried by Claude first, R11) |
|---|---|---|---|---|
| 1 | — | The owner commits this approved plan, `tools/reference/stage1/`, the decisions, the updated `docs/PROVENANCE.md` and `.sha256`, and the 1a prompt, on a new branch `stage-1-work` | | **Owner, DF1-P:** add one character to the end of `world_rules_golden.txt`. `run_checks.ps1` must stop at `[provenance]`, naming that file. Then restore it with `git checkout`. |
| 2 | **1a** | position, action, world_state, world_text (parse and render); test layouts, golden-file reader; CMake | P, T1–T20, RT1–3, V1–V6, N1, G2 (105) | **Cursor, DF1a-1:** remove `y == 9` from the border-cell test. Exactly T8 and G2 must fail. **Cursor, DF1a-2:** make `isWall` return false outside the grid. Exactly V6a, V6b, V6c and V6d must fail. |
| 3 | **1b** | rules, environment; the shared bounds-safe golden readers | R1–R11, N2, E1–E4, G1, GN1–GN6, GR1–GR5 (139) | **Cursor, DF1b-1:** a push into a wall still moves the agent (the original Stage 1 check). Exactly R4 and G1 must fail (and D1 too, once it exists in 1c). **Cursor, DF1b-2:** remove the parse reader's `expect ` prefix check. Exactly GN3 must fail. |
| 4 | **1c** | scripted demo, main, `currentStage()` = 1, README | D1 (140) | **Cursor, DF1c-1:** the demo uses four East actions. Exactly D1 must fail. **Owner, DF1c-2:** in `src/world/rules.cpp`, make the Pushed case return Moved. `run_checks.ps1` must stop at `[tests]`, with the FAIL lines of R3, R5, G1 and D1 visible. |
| 5 | — | Fresh clone of `stage-1-work`; `run_checks.ps1` passes there | | |
| 6 | — | Completion gate (Section 10), ChatGPT's review of the complete source, tag `stage-1-v2`, merge into `main` | | |

Each sub-stage ends with the owner's commit on `stage-1-work`, after Claude's review.

**DF1c-2 prediction, from the reference:** with Pushed reported as Moved, these checks fail:
- R3 and R5 (they check Pushed);
- G1 (230 golden lines say Pushed);
- D1 (steps 2–4 say Pushed).

No other check fails: R8b checks only walls and R9 only the input. This prediction was tried on the reference with `df_check.py`.

## 8. Decisions to record (with the plan's commit, R12)
| ID | Decision | Reason |
|---|---|---|
| D-060 | The rebuilt world reproduces the original world (`src/world` at `7587f38`) exactly, including the order of checks and the error messages. Namespace `aih`. `world_text.cpp` is split into `world_parse.cpp` and `world_render.cpp`. | The C1 baseline must later reproduce the original step for step (M1); the world is its ground truth. |
| D-061 | Golden world values are produced once from the original C++ (`orig_trace/`) and committed with their commit, their SHA-256 values, and the tool's source. Tests G1 and G2 must reproduce them. `run_checks.ps1` verifies their hashes as preserved files, and those of the 13 original source files the tool was built from (v1.1). They are early evidence of equivalence, not the M1.4 gate itself. | R20's method (golden values from the original code, stored with commit and hash), applied to the first component that can be compared. |
| D-062 | The program's header lines say "AI Hologram v2 -- Stage 1"; the demo part is byte-identical to the original. | The program text is not part of the equivalence gate. "v2" avoids confusion with the original. |
| D-063 | Tests find data files through the compile definition `AIH_SOURCE_DIR` (the repository root). | No hard-coded paths, so a fresh clone anywhere works. |

## 9. Documentation
- **Headers.** Every new file has the three-part header: Purpose / "Introduced in Stage 1x (rebuild)" / Architecture.
- **Beginner comments.** They explain each C++ feature the first time it appears, as the original plan 9.1 listed:
  - `struct` versus `class`, and `private`;
  - constructors;
  - `enum class` and `switch`;
  - `std::array` and 2-D arrays;
  - `std::optional`;
  - operator overloading;
  - `const` member functions;
  - passing by `const` reference;
  - `std::ostream&` as a parameter.
- **README** (1c). The status becomes "Stage 1 of the rebuild: the grid world (movement rules, text format, scripted demonstration), checked against golden values from the original code. No reasoning components exist yet. No experiment has been run."
- **Handoff notes.** One per sub-stage.

## 10. Completion checklist (owner verification gate)
- [ ] The plan was approved and committed before any Stage 1 code.
- [ ] `run_checks.ps1` passes on the owner's computer **and on a fresh clone**, with the exact final line of Section 5.
- [ ] The program output is exactly 3.8; the tests show `129 checks, 0 failures`; CTest 2/2; 0 checker violations.
- [ ] G1 and G2 pass: the rebuilt world agrees with the original on all 5,184 rule cases and all 1,009 parser cases.
- [ ] All the deliberate failure checks gave exactly the predicted failures.
- [ ] D-060 to D-063 recorded; handoff notes appended; prompts saved.
- [ ] The owner can explain Section 11.

## 11. Things the owner should understand by the end
1. How (x, y) maps to the text, and why walls are stored as `walls[y][x]`.
2. Why `applyAction` returns a new state instead of changing the old one (predictions later), and why only `Environment` holds the true state.
3. What a golden value is, why it came from the original code, and why its hash is checked.
4. Why the order of the parser's checks matters for "exactly like the original".
5. Why blocked actions still count as steps.

## 12. What actually happened (filled in at the end)
```
Date completed:
Commit tagged stage-1-v2:
Checks:
Did everything go as planned?
```

## 13. Instructions to the coding assistant
Every prompt says:
- follow `AGENTS.md` and `.cursor/rules`;
- implement only the named sub-stage;
- read the reference (`tools/reference/stage1/ref_world.py` and `checks.py`) for exact behaviour and test data;
- never commit, and never change an expected value;
- if a golden case fails, report the case and both texts, and change nothing to hide it;
- format with CLion's clang-format;
- run `run_checks.ps1` and report its final line exactly.

## 14. Open points for review
- **O1. Settled by ChatGPT:** the golden-file sizes are accepted. Do not reduce the cases.
- **O2. Settled by ChatGPT:** the original source files stay under `tools/reference/stage1/orig_trace/original_src/`, and are also in the protected provenance list (v1.1).

## Revision history
| Version | Date | Change |
|---|---|---|
| 1.0 | 2026-10-20 | First draft. The reference, the check mirror (58 checks) and the deliberate failure predictions were built first (R9, R11). The golden values were produced from the original C++ and reproduced byte for byte by the reference. |
| 1.1 | 2026-10-20 | ChatGPT approved v1.0. It ran the 58 checks and the five deliberate failure checks, compiled the original C++, and regenerated both golden files byte for byte. It made one provenance improvement: the 13 original source files go into `docs/PROVENANCE.sha256` too, each hash verified against `7587f38`. Preserved files 85 → 98 (Section 5, D-061). O1 and O2 settled. No change to the world rules, the tests or the golden values. |
| 1.2 | 2026-10-20 | After Stage 1a (all 105 checks passed; Claude also compared the new parser with the original on 20,000 fresh random inputs: identical). (1) G1 goes in its own file, `tests/test_world_golden_rules.cpp`, because `test_world_golden.cpp` already had 135 lines with G2. (2) Claude's review found that 1a's G2 reader could read past the end of a truncated file. That cannot happen while the hash check passes, but the tests must be safe on their own. So both readers become shared, bounds-safe functions (`golden_readers.h`, `golden_parse_reader.cpp`, `golden_rules_reader.cpp`) with exact problem messages (6.7). (3) ChatGPT's safeguard: 11 malformed-file tests GN1–GN6 and GR1–GR5, with exact expected results from `ref_golden_readers.py`, plus deliberate failure DF1b-2. Checks: 1b 128 → 139, 1c 129 → 140. Files checked: 1b 54 → 58, 1c 57 → 61. No change to the world, the golden values or the earlier checks. |
