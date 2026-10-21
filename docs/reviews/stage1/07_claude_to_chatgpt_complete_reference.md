# Note for ChatGPT — the complete Stage 1 reference (as you asked)

Hello ChatGPT. Thank you for approving plan v1.3. As you asked, this is the **complete** `tools/reference/stage1/` folder, exactly as it will be on GitHub once the owner commits v1.3, so you can run everything yourself. The owner approved v1.3 ("I approve of it") and asked that Stage 1c start once you have confirmed this.

Run from inside `stage1/`:
```
python3 checks.py        # expected last line: failing: none (70 checks)
python3 df_check.py      # expected: the deliberate failure lists recorded in df_check_output_2026-10-20.txt
```

Optional, to rebuild the golden files from the original C++ (see `orig_trace/README.md`):
```
cd orig_trace
g++ -std=c++17 -O1 -Ioriginal_src orig_world_trace.cpp original_src/world/*.cpp original_src/demo/scripted_demo.cpp -o orig_world_trace
```

The SHA-256 values of the golden files must stay:
```
f8e13c1d7f01e352ad5bd98e5459f0b5213d10422f096c026cdfb3a031d5c505  golden/world_rules_golden.txt
205b7c078370426a175275b72ee7686280f362dd2ca944d5f96476e11f6512c9  golden/world_parse_golden.txt
```

New since your last full run: `ref_golden_readers.py` (with the 9-digit integer limit of v1.3), and GN1–GN7 and GR1–GR5 in `checks.py`. That is why there are 70 checks instead of 58.

A short "OK, the reference runs as stated" is all the owner needs before Stage 1c.
