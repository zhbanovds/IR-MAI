import os
import json

# Папки/файлы полного корпуса
CORPUS_DIR = os.path.join("corpus", "arxiv_ml")
META_PATH = os.path.join(CORPUS_DIR, "metadata.jsonl")
TEXT_DIR = os.path.join(CORPUS_DIR, "text")

# Сколько документов проверяем (быстро и достаточно)
CHECK_DOCS = 200

# Минимальный размер txt (байты), чтобы не считать совсем пустые файлы валидными
MIN_SIZE_BYTES = 50


def _iter_meta_first_n(n: int):
    with open(META_PATH, "r", encoding="utf-8") as f:
        for i, line in enumerate(f, start=1):
            if i > n:
                break
            line = line.strip()
            if not line:
                continue
            yield json.loads(line)


def test_text_dir_exists():
    assert os.path.isdir(TEXT_DIR), f"text dir not found: {TEXT_DIR}"


def test_text_files_exist_for_first_n_docs():
    assert os.path.isfile(META_PATH), f"metadata.jsonl not found: {META_PATH}"

    missing = []
    too_small = []

    for rec in _iter_meta_first_n(CHECK_DOCS):
        doc_id = rec.get("doc_id")
        assert doc_id is not None, "doc_id missing in metadata record"

        fname = f"{int(doc_id):06d}.txt"
        path = os.path.join(TEXT_DIR, fname)

        if not os.path.exists(path):
            missing.append(fname)
            continue

        size = os.path.getsize(path)
        if size < MIN_SIZE_BYTES:
            too_small.append((fname, size))

    assert not missing, f"Missing text files (first {CHECK_DOCS} docs): {missing[:10]} (and more)" if len(missing) > 10 else f"Missing text files: {missing}"
    assert not too_small, f"Too small text files (first {CHECK_DOCS} docs): {too_small[:10]} (and more)" if len(too_small) > 10 else f"Too small text files: {too_small}"