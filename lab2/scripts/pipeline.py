import os
import sys
import argparse
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser()
parser.add_argument("--meta", action="store_true", help="run metadata step (lab1 01)")
parser.add_argument("--pdf", action="store_true", help="run pdf download step (lab1 02)")
parser.add_argument("--txt", action="store_true", help="run pdf->txt step (lab1 03)")
parser.add_argument("--all", action="store_true", help="run all steps: meta->pdf->txt")
parser.add_argument("--python", default=sys.executable, help="python executable to use")
args = parser.parse_args()

root = Path(__file__).resolve().parents[2]
lab1_scripts = root / "lab1" / "scripts"

step_meta = lab1_scripts / "01_fetch_meta_arxiv.py"
step_pdf = lab1_scripts / "02_download_pdfs.py"
step_txt = lab1_scripts / "03_pdf_to_text.py"

for p in (step_meta, step_pdf, step_txt):
    if not p.exists():
        print(f"ERROR: missing script: {p}")
        sys.exit(1)

run_any = args.all or args.meta or args.pdf or args.txt
if not run_any:
    print("ERROR: choose one: --meta/--pdf/--txt/--all")
    sys.exit(1)

def run(cmd):
    print(f"$ {' '.join(map(str, cmd))}")
    r = subprocess.run(cmd)
    if r.returncode != 0:
        sys.exit(r.returncode)

if args.all or args.meta:
    run([args.python, str(step_meta)])

if args.all or args.pdf:
    run([args.python, str(step_pdf)])

if args.all or args.txt:
    run([args.python, str(step_txt)])