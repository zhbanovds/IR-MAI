import os
import subprocess
from tqdm import tqdm

def main():
    base_dir = os.path.join("corpus", "arxiv_ml")
    pdf_dir = os.path.join(base_dir, "raw_pdf")
    text_dir = os.path.join(base_dir, "text")
    os.makedirs(text_dir, exist_ok=True)

    pdfs = sorted([f for f in os.listdir(pdf_dir) if f.endswith(".pdf")])
    pbar = tqdm(pdfs, desc="PDF -> TXT")

    for pdf in pbar:
        doc_id = os.path.splitext(pdf)[0]
        in_path = os.path.join(pdf_dir, pdf)
        out_path = os.path.join(text_dir, f"{doc_id}.txt")

        if os.path.exists(out_path) and os.path.getsize(out_path) > 1024:
            continue

        # -enc UTF-8 чтобы сразу было как требует курс
        try:
            subprocess.run(
                ["pdftotext", "-enc", "UTF-8", in_path, out_path],
                check=True,
                stdout=subprocess.DEVNULL,
                stderr=subprocess.DEVNULL,
            )
        except Exception:
            # если конкретный pdf битый/нестандартный — пропускаем
            continue

    pbar.close()
    print(f"text dir: {text_dir}")

if __name__ == "__main__":
    main()