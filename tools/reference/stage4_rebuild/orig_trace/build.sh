#!/bin/sh
# Builds the Stage 4 golden-trace tool from the ORIGINAL C++ only (commit 89ea9b6), and
# checks that it reproduces the committed golden file byte for byte. Run from this
# folder (Linux, g++).
#   orig_fault_trace   src/agent at 89ea9b6 (stage-3 plus IE-1; ../../stage3/orig_trace),
#                      src/experiment at 89ea9b6 (original_src/experiment)
set -e
W=../../stage1/orig_trace/original_src
M=../../stage2/orig_trace/original_src
A=../../stage3/orig_trace
OUT=${1:-/tmp/stage4_orig_tools}
mkdir -p "$OUT"
AG=""
for f in $A/original_src/agent/*.cpp; do
    b=$(basename "$f")
    if [ -f "$A/original_src_ie1/agent/$b" ]; then AG="$AG $A/original_src_ie1/agent/$b"; else AG="$AG $f"; fi
done
g++ -std=c++17 -O2 -DWITH_OBSERVER -I$W -I$M -I$A/original_src_ie1 -I$A/original_src -Ioriginal_src \
    orig_fault_trace.cpp $W/world/*.cpp $M/model/*.cpp $M/compare/*.cpp $AG original_src/experiment/*.cpp \
    -o "$OUT/orig_fault_trace"
if [ -f ../golden/fault_cases.txt ]; then
    "$OUT/orig_fault_trace" traces ../golden/fault_cases.txt | cmp - ../golden/fault_traces.txt
    echo "golden file reproduced: fault traces (89ea9b6)"
fi
