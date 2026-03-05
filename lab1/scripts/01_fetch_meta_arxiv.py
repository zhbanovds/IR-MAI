import os, json, time
import requests
import xml.etree.ElementTree as ET
from tqdm import tqdm

BASE = "https://export.arxiv.org/api/query"
NS = {"atom": "http://www.w3.org/2005/Atom"}

def safe_text(elem, path):
    x = elem.find(path, NS)
    return (x.text or "").strip() if x is not None else ""

def fetch_batch(search_query: str, start: int, max_results: int):
    params = {
        "search_query": search_query,
        "start": start,
        "max_results": max_results,
        "sortBy": "submittedDate",
        "sortOrder": "descending",
    }
    r = requests.get(BASE, params=params, timeout=60)
    r.raise_for_status()
    return r.text

def parse_entries(xml_text: str):
    root = ET.fromstring(xml_text)
    entries = root.findall("atom:entry", NS)
    out = []
    for e in entries:
        entry_id = safe_text(e, "atom:id")          # https://arxiv.org/abs/XXXX.XXXXXvN
        title = safe_text(e, "atom:title")
        published = safe_text(e, "atom:published")
        summary = safe_text(e, "atom:summary")

        # categories
        cats = []
        for c in e.findall("atom:category", NS):
            term = c.attrib.get("term")
            if term:
                cats.append(term)

        # arxiv_id from URL
        arxiv_id = entry_id.rsplit("/", 1)[-1] if entry_id else ""

        pdf_url = f"https://arxiv.org/pdf/{arxiv_id}.pdf" if arxiv_id else ""
        abs_url = f"https://arxiv.org/abs/{arxiv_id}" if arxiv_id else ""

        out.append({
            "arxiv_id": arxiv_id,
            "title": " ".join(title.split()),
            "published": published,
            "categories": cats,
            "abs_url": abs_url,
            "pdf_url": pdf_url,
            "abstract": " ".join(summary.split()),
        })
    return out

def main():
    # ТЕМАТИКА: Machine Learning
    # Можно оставить только cs.LG, чтобы было едино.
    search_query = "cat:cs.LG"
    target = 5006             # на "удовлетворительно" достаточно 30k
    batch = 200                # arXiv API нормально переваривает 100-200
    sleep_sec = 3              # чтобы не долбить API

    out_dir = os.path.join("corpus", "arxiv_ml")
    os.makedirs(out_dir, exist_ok=True)
    meta_path = os.path.join(out_dir, "metadata.jsonl")

    # resume: если файл уже есть — продолжаем
    existing = 0
    if os.path.exists(meta_path):
        with open(meta_path, "r", encoding="utf-8") as f:
            existing = sum(1 for _ in f)

    start = existing
    doc_id = existing + 1

    pbar = tqdm(total=target, initial=existing, desc="Fetching metadata")
    while existing < target:
        xml_text = fetch_batch(search_query, start=start, max_results=batch)
        entries = parse_entries(xml_text)
        if not entries:
            break

        with open(meta_path, "a", encoding="utf-8") as f:
            for it in entries:
                if existing >= target:
                    break
                rec = {
                    "doc_id": doc_id,
                    **it
                }
                f.write(json.dumps(rec, ensure_ascii=False) + "\n")
                existing += 1
                doc_id += 1
                pbar.update(1)

        start += len(entries)
        time.sleep(sleep_sec)

    pbar.close()
    print(f"Done. metadata lines: {existing}")
    print(f"Saved: {meta_path}")

if __name__ == "__main__":
    main()