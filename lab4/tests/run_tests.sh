#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LOG="$ROOT/tests/test_output.txt"

echo "lab4 stemming test" | tee "$LOG"

mkdir -p "$ROOT/build"
cmake -S "$ROOT" -B "$ROOT/build" 2>&1 | tee -a "$LOG"
cmake --build "$ROOT/build" -j 2>&1 | tee -a "$LOG"

TOKENS="$ROOT/tests/sample_tokens.tsv"
OUT="$ROOT/tests/stems.tsv"

cat > "$TOKENS" << 'EOF'
000001	organizations
000001	organizational
000001	organization
000001	processing
000001	processed
000001	processor
000001	running
000001	runs
000001	happily
000001	classes
000001	class
EOF

"$ROOT/build/stem" --in "$TOKENS" --out "$OUT" 2>&1 | tee -a "$LOG"

LINES=$(wc -l < "$OUT" | tr -d ' ')
echo "stems_lines=$LINES" | tee -a "$LOG"

if [ "$LINES" -le 0 ]; then
  echo "FAIL: empty output" | tee -a "$LOG"
  exit 1
fi

head -n 50 "$OUT" | tee -a "$LOG"
echo "OK" | tee -a "$LOG"