import os
import sys
import json

MIN_SIZE_BYTES = 50
CHECK_DOCS = 300

corpus_dir = sys.argv[1] if len(sys.argv) > 1 else "corpus/arxiv_ml"
meta_path = os.path.join(corpus_dir, "metadata.jsonl")
text_dir = os.path.join(corpus_dir, "text")

print(f"corpus_dir={corpus_dir}")
print(f"meta_path={meta_path}")
print(f"text_dir={text_dir}")

if not os.path.isfile(meta_path):
    print("ERROR: metadata.jsonl not found or not a file")
    sys.exit(1)

if not os.path.isdir(text_dir):
    print("ERROR: text/ directory not found")
    sys.exit(1)

required = {"doc_id", "arxiv_id", "title", "published", "categories", "abs_url", "pdf_url"}

total_lines = 0
bad_json = 0
missing_fields = 0
missing_txt = 0
too_small = 0

with open(meta_path, "r", encoding="utf-8") as f:
    for line_no, line in enumerate(f, start=1):
        line = line.strip()
        if not line:
            continue
        total_lines += 1
        if total_lines > CHECK_DOCS:
            continue
        try:
            rec = json.loads(line)
        except Exception:
            bad_json += 1
            continue
        if not required.issubset(rec.keys()):
            missing_fields += 1
        doc_id = rec.get("doc_id")
        try:
            doc_id_i = int(doc_id)
        except Exception:
            missing_txt += 1
            continue
        txt_name = f"{doc_id_i:06d}.txt"
        txt_path = os.path.join(text_dir, txt_name)
        if not os.path.exists(txt_path):
            missing_txt += 1
            continue
        if os.path.getsize(txt_path) < MIN_SIZE_BYTES:
            too_small += 1

print(f"metadata_lines_total={total_lines}")
print(f"checked_first={min(total_lines, CHECK_DOCS)}")
print(f"bad_json_firstN={bad_json}")
print(f"missing_required_fields_firstN={missing_fields}")
print(f"missing_txt_firstN={missing_txt}")
print(f"too_small_txt_firstN(<{MIN_SIZE_BYTES}B)={too_small}")

if bad_json > 0:
    print("ERROR: invalid JSON lines detected in the first chunk")
    sys.exit(1)

print("VERIFY_OK")