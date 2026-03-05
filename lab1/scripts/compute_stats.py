import os
import sys

def human_bytes(n: int) -> str:
    units = ["B", "KB", "MB", "GB", "TB"]
    x = float(n)
    for u in units:
        if x < 1024.0:
            return f"{x:.2f} {u}"
        x /= 1024.0
    return f"{x:.2f} PB"

corpus_dir = sys.argv[1] if len(sys.argv) > 1 else "corpus/arxiv_ml"
meta_path = os.path.join(corpus_dir, "metadata.jsonl")
text_dir = os.path.join(corpus_dir, "text")

meta_lines = 0
if os.path.isfile(meta_path):
    with open(meta_path, "r", encoding="utf-8") as f:
        for line in f:
            if line.strip():
                meta_lines += 1

txt_files = []
if os.path.isdir(text_dir):
    txt_files = [os.path.join(text_dir, x) for x in os.listdir(text_dir) if x.endswith(".txt")]

sizes = []
for p in txt_files:
    try:
        sizes.append(os.path.getsize(p))
    except OSError:
        pass

total = sum(sizes)
count = len(sizes)
avg = (total / count) if count else 0
mn = min(sizes) if sizes else 0
mx = max(sizes) if sizes else 0

print(f"meta_lines={meta_lines}")
print(f"text_files={count}")
print(f"text_total_size_bytes={total}")
print(f"text_total_size_human={human_bytes(total)}")
print(f"text_avg_size_bytes={avg:.2f}")
print(f"text_min_size_bytes={mn}")
print(f"text_max_size_bytes={mx}")