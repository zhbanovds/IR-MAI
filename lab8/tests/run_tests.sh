#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LOG="$ROOT/tests/test_output.txt"

echo "lab8 boolean search test" | tee "$LOG"

mkdir -p "$ROOT/build"
cmake -S "$ROOT" -B "$ROOT/build" 2>&1 | tee -a "$LOG"
cmake --build "$ROOT/build" -j 2>&1 | tee -a "$LOG"

TMP="$ROOT/tests/tmp"
IDX="$TMP/index"
rm -rf "$TMP"
mkdir -p "$IDX"

cat > "$IDX/dict.tsv" << 'EOF'
0	deep
1	learning
2	model
EOF

python3 - << PY
import struct, os
idx = r"$IDX"
offs = [0, 12, 24]
with open(os.path.join(idx, "offsets.bin"), "wb") as f:
    for o in offs:
        f.write(struct.pack("<Q", o))
with open(os.path.join(idx, "postings.bin"), "wb") as f:
    f.write(struct.pack("<I", 2)); f.write(struct.pack("<II", 2, 3))  # deep -> [2,3]
    f.write(struct.pack("<I", 2)); f.write(struct.pack("<II", 1, 3))  # learning -> [1,3]
    f.write(struct.pack("<I", 2)); f.write(struct.pack("<II", 1, 2))  # model -> [1,2]
PY

echo "--- query: deep AND learning ---" | tee -a "$LOG"
"$ROOT/build/search" --index_dir "$IDX" --query "deep AND learning" 2>&1 | tee -a "$LOG"

echo "--- query: model OR deep ---" | tee -a "$LOG"
"$ROOT/build/search" --index_dir "$IDX" --query "model OR deep" 2>&1 | tee -a "$LOG"

echo "--- query: learning AND NOT model ---" | tee -a "$LOG"
"$ROOT/build/search" --index_dir "$IDX" --query "learning AND NOT model" --limit 20 2>&1 | tee -a "$LOG"

echo "OK" | tee -a "$LOG"