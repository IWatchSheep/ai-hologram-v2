# Stage 2 reference (rules R9, R10, R11, R20)

Claude's Python reference for the rebuilt prediction and comparison code, built and run before plan `docs/stages/STAGE_02.md` was reviewed. It uses the Stage 1 reference in `../stage1/` (the world, the shared reader helpers, and the test layouts).

| File | What it is |
|---|---|
| `ref_model.py` | `BudgetCounter`, `WorldModel.predict`, `compare_states`, `compare_prediction`, `matches`, `describe_differences`, and the prediction demo text. It follows the original C++ (`src/model`, `src/compare`, `src/demo/prediction_demo` at `7587f38`). |
| `ref_prediction_reader.py` | The prediction-golden reader (plan 6.5): the exact order of checks and problem messages the C++ reader follows. |
| `checks.py` | The plan's 36 new checks, mirrored with their C++ ids: `failing: none (36 checks)`. |
| `df_check.py` | The deliberate failure checks tried on the reference first; recorded output in `df_check_output_2026-10-20.txt`. |
| `gen_golden.py` | Writes the 1,000 golden cases (seed 20261021), and computes the golden file from the reference. |
| `golden/prediction_golden.txt` | **The golden values, produced by the original C++** (`orig_trace/`). The reference reproduces them byte for byte; the SHA-256 is in `docs/PROVENANCE.sha256`. |
| `orig_trace/` | The golden tool's source, and the 12 original files it was built from (see its README). |

Run the scripts from this folder:
- `python3 checks.py`
- `python3 df_check.py`

The coding assistant never edits anything here.
