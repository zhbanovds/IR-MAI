#!/usr/bin/env bash
set -euo pipefail

CORPUS_DIR="${1:-corpus/arxiv_ml}"

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
REPO_ROOT="$(cd "$ROOT/.." && pwd)"
LOG="$ROOT/tests/test_output.txt"

echo "Corpus: $CORPUS_DIR" | tee "$LOG"

echo "--- lab2 pipeline.py --meta ---" | tee -a "$LOG"
python3 "$REPO_ROOT/lab2/scripts/pipeline.py" --meta 2>&1 | tee -a "$LOG"

echo "--- lab1 verify_corpus.py ---" | tee -a "$LOG"
python3 "$REPO_ROOT/lab1/scripts/verify_corpus.py" "$CORPUS_DIR" 2>&1 | tee -a "$LOG"

echo "--- lab1 compute_stats.py ---" | tee -a "$LOG"
python3 "$REPO_ROOT/lab1/scripts/compute_stats.py" "$CORPUS_DIR" 2>&1 | tee -a "$LOG"

echo "OK" | tee -a "$LOG"