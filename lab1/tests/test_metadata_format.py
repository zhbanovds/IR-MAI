import os
import json

# Настройки (можешь поменять при необходимости)
CORPUS_DIR = os.path.join("corpus", "arxiv_ml")
META_PATH = os.path.join(CORPUS_DIR, "metadata.jsonl")

# Сколько строк проверяем (быстро и достаточно для автотеста)
CHECK_LINES = 200


def test_metadata_file_exists():
    assert os.path.exists(META_PATH), f"metadata.jsonl not found: {META_PATH}"
    assert os.path.isfile(META_PATH), f"metadata.jsonl is not a file: {META_PATH}"


def test_metadata_jsonl_format_and_required_fields():
    required = {"doc_id", "arxiv_id", "title", "published", "categories", "abs_url", "pdf_url"}

    with open(META_PATH, "r", encoding="utf-8") as f:
        for i, line in enumerate(f, start=1):
            if i > CHECK_LINES:
                break

            line = line.strip()
            assert line, f"Empty line at {i}"

            try:
                rec = json.loads(line)
            except json.JSONDecodeError as e:
                raise AssertionError(f"Invalid JSON at line {i}: {e}") from e

            missing = required - set(rec.keys())
            assert not missing, f"Missing fields at line {i}: {sorted(missing)}"

            # базовые проверки типов/значений
            assert isinstance(rec["doc_id"], int), f"doc_id must be int at line {i}"
            assert rec["doc_id"] > 0, f"doc_id must be > 0 at line {i}"

            assert isinstance(rec["arxiv_id"], str) and rec["arxiv_id"], f"arxiv_id empty at line {i}"
            assert isinstance(rec["title"], str), f"title must be str at line {i}"
            assert isinstance(rec["published"], str) and rec["published"], f"published empty at line {i}"

            assert isinstance(rec["categories"], list), f"categories must be list at line {i}"
            assert isinstance(rec["abs_url"], str) and rec["abs_url"].startswith("https://"), f"abs_url invalid at line {i}"
            assert isinstance(rec["pdf_url"], str) and rec["pdf_url"].startswith("https://"), f"pdf_url invalid at line {i}"


def test_doc_id_is_monotonic_for_first_chunk():
    prev = 0
    with open(META_PATH, "r", encoding="utf-8") as f:
        for i, line in enumerate(f, start=1):
            if i > CHECK_LINES:
                break
            rec = json.loads(line)
            cur = rec["doc_id"]
            assert cur > prev, f"doc_id not increasing at line {i}: prev={prev}, cur={cur}"
            prev = cur