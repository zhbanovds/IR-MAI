#!/usr/bin/env python3
import os
import json
import shutil
import argparse

parser = argparse.ArgumentParser(
    description="Create a small corpus_sample/ subset from the full corpus for GitHub."
)
parser.add_argument(
    "--corpus_dir",
    default="corpus/arxiv_ml",
    help="Path to full corpus dir containing metadata.jsonl and text/. (default: corpus/arxiv_ml)",
)
parser.add_argument(
    "--out_dir",
    default="corpus_sample",
    help="Output directory for sample corpus (default: corpus_sample)",
)
parser.add_argument(
    "--limit",
    type=int,
    default=200,
    help="How many documents to copy into sample (default: 200)",
)
parser.add_argument(
    "--clean",
    action="store_true",
    help="If set, remove out_dir before creating sample.",
)

args = parser.parse_args()

corpus_dir = args.corpus_dir
meta_path = os.path.join(corpus_dir, "metadata.jsonl")
text_dir = os.path.join(corpus_dir, "text")

if not os.path.exists(meta_path):
    raise FileNotFoundError(f"metadata.jsonl not found: {meta_path}")
if not os.path.isdir(text_dir):
    raise FileNotFoundError(f"text/ dir not found: {text_dir}")

out_dir = args.out_dir
out_text_dir = os.path.join(out_dir, "text")
out_meta_path = os.path.join(out_dir, "metadata.jsonl")

if args.clean and os.path.exists(out_dir):
    shutil.rmtree(out_dir)

os.makedirs(out_text_dir, exist_ok=True)

def iter_meta(path: str):
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            line = line.strip()
            if line:
                yield json.loads(line)

copied = 0
written = 0

with open(out_meta_path, "w", encoding="utf-8") as out_meta:
    for rec in iter_meta(meta_path):
        if written >= args.limit:
            break

        doc_id = rec.get("doc_id")
        if doc_id is None:
            continue

        src_txt = os.path.join(text_dir, f"{int(doc_id):06d}.txt")
        if not os.path.exists(src_txt):
            continue

        dst_txt = os.path.join(out_text_dir, f"{int(doc_id):06d}.txt")
        if not os.path.exists(dst_txt):
            shutil.copy2(src_txt, dst_txt)
            copied += 1

        out_meta.write(json.dumps(rec, ensure_ascii=False) + "\n")
        written += 1

print(f"Sample created: {out_dir}")
print(f"Metadata lines written: {written}")
print(f"Text files copied: {copied}")
print(f"Sample text dir: {out_text_dir}")
print(f"Sample metadata: {out_meta_path}")