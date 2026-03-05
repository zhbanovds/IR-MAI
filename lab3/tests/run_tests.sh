#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
LOG="$ROOT/tests/test_output.txt"

echo "lab3 tokenization test" | tee "$LOG"

mkdir -p "$ROOT/build"
cmake -S "$ROOT" -B "$ROOT/build" 2>&1 | tee -a "$LOG"
cmake --build "$ROOT/build" -j 2>&1 | tee -a "$LOG"

TMP_DIR="$ROOT/tests/tmp_docs"
OUT_TSV="$ROOT/tests/tokens.tsv"

rm -rf "$TMP_DIR"
mkdir -p "$TMP_DIR"

printf "Hello, WORLD! ML-2025: Transformers, RAG, TF-IDF.\n" > "$TMP_DIR/000001.txt"
printf "Numbers: 123 45. MixedCaseToken ABCdef. x a\n" > "$TMP_DIR/000002.txt"

"$ROOT/build/tokenize" --docs_dir "$TMP_DIR" --out "$OUT_TSV" 2>&1 | tee -a "$LOG"

LINES=$(wc -l < "$OUT_TSV" | tr -d ' ')
echo "tokens_lines=$LINES" | tee -a "$LOG"

if [ "$LINES" -le 0 ]; then
  echo "FAIL: empty output" | tee -a "$LOG"
  exit 1
fi

head -n 20 "$OUT_TSV" | tee -a "$LOG"
echo "OK" | tee -a "$LOG"