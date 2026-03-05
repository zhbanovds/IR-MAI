#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LOG="$ROOT/tests/test_output.txt"

echo "lab5 zipf test" | tee "$LOG"

mkdir -p "$ROOT/build"
cmake -S "$ROOT" -B "$ROOT/build" 2>&1 | tee -a "$LOG"
cmake --build "$ROOT/build" -j 2>&1 | tee -a "$LOG"

IN="$ROOT/tests/sample_tokens.tsv"
FREQ="$ROOT/tests/freq.tsv"
ZIPF="$ROOT/tests/zipf.tsv"

cat > "$IN" << 'EOF'
000001	the
000001	the
000001	the
000001	model
000001	model
000001	learning
000002	the
000002	learning
000002	learning
000002	learning
000002	model
EOF

"$ROOT/build/zipf" --in "$IN" --out "$FREQ" --zipf "$ZIPF" --top 10 2>&1 | tee -a "$LOG"

LINES_FREQ=$(wc -l < "$FREQ" | tr -d ' ')
LINES_ZIPF=$(wc -l < "$ZIPF" | tr -d ' ')
echo "freq_lines=$LINES_FREQ" | tee -a "$LOG"
echo "zipf_lines=$LINES_ZIPF" | tee -a "$LOG"

if [ "$LINES_FREQ" -le 0 ] || [ "$LINES_ZIPF" -le 0 ]; then
  echo "FAIL: empty output" | tee -a "$LOG"
  exit 1
fi

echo "--- top output ---" | tee -a "$LOG"
"$ROOT/build/zipf" --in "$IN" --out "$FREQ" --top 10 | tee -a "$LOG"

echo "--- freq.tsv (first 20) ---" | tee -a "$LOG"
head -n 20 "$FREQ" | tee -a "$LOG"

echo "--- zipf.tsv (first 20) ---" | tee -a "$LOG"
head -n 20 "$ZIPF" | tee -a "$LOG"

echo "OK" | tee -a "$LOG"