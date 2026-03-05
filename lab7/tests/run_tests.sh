#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LOG="$ROOT/tests/test_output.txt"

echo "lab7 indexer test" | tee "$LOG"

mkdir -p "$ROOT/build"
cmake -S "$ROOT" -B "$ROOT/build" 2>&1 | tee -a "$LOG"
cmake --build "$ROOT/build" -j 2>&1 | tee -a "$LOG"

TMP="$ROOT/tests/tmp"
IN="$TMP/stems.tsv"
OUT_DIR="$TMP/index_out"

rm -rf "$TMP"
mkdir -p "$TMP"

cat > "$IN" << 'EOF'
000001	learning
000001	model
000001	model
000002	learning
000002	deep
000002	model
000003	deep
000003	learning
EOF

"$ROOT/build/indexer" --in "$IN" --out_dir "$OUT_DIR" 2>&1 | tee -a "$LOG"

ls -la "$OUT_DIR" | tee -a "$LOG"

test -f "$OUT_DIR/dict.tsv"
test -f "$OUT_DIR/postings.bin"
test -f "$OUT_DIR/offsets.bin"

echo "--- dict.tsv (first 20) ---" | tee -a "$LOG"
head -n 20 "$OUT_DIR/dict.tsv" | tee -a "$LOG"

echo "OK" | tee -a "$LOG"