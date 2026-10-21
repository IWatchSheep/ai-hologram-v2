# Stage 1 reference (rules R9, R10, R11, R20)

Claude's Python reference for the rebuilt grid world, built and run before plan `docs/stages/STAGE_01.md` was reviewed.

| File | What it is |
|---|---|
| `ref_world.py` | The world: rules, parser (same order of checks and the same messages as the original), renderer, validity, `Environment`, and the scripted demo text. It follows the original C++ data flow (`src/world` at `7587f38`). |
| `checks.py` | The plan's checks (Section 6) mirrored on the reference, each with its C++ id. It prints `failing: none (69 checks)`; N1 and N2 are placeholders, because Python has no enum names. |
| `ref_golden_readers.py` | The two golden-file readers (plan 6.7, v1.2): the exact order of checks and problem messages the C++ readers follow, and the malformed-file cases GN1–GN6 and GR1–GR5 (in `checks.py`). |
| `df_check.py` | The deliberate failure checks tried on the reference first, with their exact failing ids (recorded in `df_check_output_2026-10-20.txt`). |
| `gen_golden.py` | Writes the golden case lists, and computes the golden files from the reference. |
| `golden/` | **The golden values, produced by the original C++** (`orig_trace/`). The reference reproduces them byte for byte. Their SHA-256 values are in `docs/PROVENANCE.sha256`. |
| `orig_trace/` | The golden tool's source, and the original files it was built from (see its README). |

Run the scripts from this folder:
- `python3 checks.py`
- `python3 df_check.py`

The coding assistant never edits anything here. A disagreement between the C++ and a golden value is reported, never "fixed" by changing an expected value (R22).
