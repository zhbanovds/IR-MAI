import os, json, time
import requests
from tqdm import tqdm

def iter_meta(meta_path):
    with open(meta_path, "r", encoding="utf-8") as f:
        for line in f:
            if line.strip():
                yield json.loads(line)

def download(url, out_path, timeout=120):
    r = requests.get(url, stream=True, timeout=timeout)
    r.raise_for_status()
    tmp = out_path + ".part"
    with open(tmp, "wb") as f:
        for chunk in r.iter_content(chunk_size=1024 * 256):
            if chunk:
                f.write(chunk)
    os.replace(tmp, out_path)

def main():
    base_dir = os.path.join("corpus", "arxiv_ml")
    meta_path = os.path.join(base_dir, "metadata.jsonl")
    pdf_dir = os.path.join(base_dir, "raw_pdf")
    os.makedirs(pdf_dir, exist_ok=True)

    items = list(iter_meta(meta_path))
    pbar = tqdm(items, desc="Downloading PDFs")

    for it in pbar:
        doc_id = it["doc_id"]
        arxiv_id = it.get("arxiv_id", "")
        url = it.get("pdf_url", "")
        if not url or not arxiv_id:
            continue

        name = f"{doc_id:06d}.pdf"
        out_path = os.path.join(pdf_dir, name)
        if os.path.exists(out_path) and os.path.getsize(out_path) > 1024:
            continue

        try:
            download(url, out_path)
            time.sleep(1)  # щадим сервер
        except Exception as e:
            # пропускаем, потом можно перезапустить — докачает
            pbar.set_postfix_str(f"err doc_id={doc_id}")
            continue

    pbar.close()
    print(f"PDF dir: {pdf_dir}")

if __name__ == "__main__":
    main()
