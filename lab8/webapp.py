import html
import json
import subprocess
from pathlib import Path

from flask import Flask, request

app = Flask(__name__)

REPO_ROOT = Path(__file__).resolve().parents[1]
SEARCH_BIN = REPO_ROOT / "lab8" / "build" / "search"
DEFAULT_INDEX_DIR = str(REPO_ROOT / "lab7" / "tests" / "tmp" / "index_out")

META_JSONL = REPO_ROOT / "corpus" / "arxiv_ml" / "metadata.jsonl"
TEXT_DIR = REPO_ROOT / "corpus" / "arxiv_ml" / "text"

EXAMPLES = [
    "deep AND learning",
    "model OR deep",
    "learning AND NOT model",
    "(deep OR model) AND learning",
]

TITLE_CACHE = {}

def load_titles(limit=50000):
    if TITLE_CACHE:
        return
    if not META_JSONL.exists():
        return
    try:
        with META_JSONL.open("r", encoding="utf-8") as f:
            for i, line in enumerate(f, start=1):
                if i > limit:
                    break
                line = line.strip()
                if not line:
                    continue
                try:
                    rec = json.loads(line)
                except Exception:
                    continue
                doc_id = rec.get("doc_id")
                title = rec.get("title")
                if isinstance(doc_id, int) and isinstance(title, str) and title:
                    TITLE_CACHE[doc_id] = title
    except Exception:
        return

def run_search(index_dir: str, q: str, topk: str):
    if not SEARCH_BIN.exists():
        return None, f"search binary not found: {SEARCH_BIN}"

    cmd = [str(SEARCH_BIN), "--index_dir", index_dir, "--query", q]
    topk = (topk or "").strip()
    if topk:
        cmd += ["--limit", topk]

    try:
        p = subprocess.run(cmd, capture_output=True, text=True, timeout=30)
    except subprocess.TimeoutExpired:
        return None, "timeout: search took too long"
    except Exception as e:
        return None, f"error running search: {e}"

    if p.returncode != 0:
        err = (p.stderr or "").strip()
        out = (p.stdout or "").strip()
        return None, err or out or "search failed"

    docs = []
    for line in (p.stdout or "").splitlines():
        line = line.strip()
        if not line:
            continue
        try:
            docs.append(int(line))
        except Exception:
            continue
    return docs, None

HTML_PAGE = """<!doctype html>
<html lang="ru">
<head>
  <meta charset="utf-8">
  <title>Boolean Search (Lab08)</title>
  <style>
    body {{ font-family: -apple-system, BlinkMacSystemFont, Segoe UI, Roboto, Arial, sans-serif; margin: 20px; }}
    .title {{ font-size: 34px; font-weight: 700; margin-bottom: 14px; }}
    .row {{ display: flex; align-items: center; gap: 10px; }}
    .q {{ flex: 1; padding: 10px; font-size: 14px; }}
    .idx {{ width: 100%; padding: 10px; font-size: 13px; }}
    .topk {{ width: 80px; padding: 10px; }}
    .btn {{ padding: 10px 14px; }}
    .meta {{ color: #666; font-size: 12px; margin-top: 6px; }}
    .examples {{ margin: 14px 0; }}
    .examples ul {{ margin: 6px 0 0 18px; }}
    hr {{ border: none; border-top: 1px solid #ddd; margin: 14px 0; }}
    .shown {{ color: #444; font-size: 13px; margin: 6px 0 10px; }}
    ol {{ margin: 0 0 0 22px; }}
    li {{ margin: 8px 0; }}
    .line1 {{ font-weight: 700; font-size: 18px; }}
    .line2 {{ color: #666; font-size: 12px; }}
    code {{ background: #f2f2f2; padding: 2px 4px; border-radius: 4px; }}
    .error {{ background: #fff3f3; border: 1px solid #ffcccc; padding: 10px; border-radius: 6px; }}
    .label {{ font-weight: 600; margin: 10px 0 6px; }}
  </style>
</head>
<body>
  <div class="title">Boolean Search (Lab08)</div>

  <form method="get" action="/">
    <div class="label">Index dir:</div>
    <input class="idx" type="text" name="index_dir" value="{index_dir}">
    <div class="meta">Папка с <code>dict.tsv</code>, <code>offsets.bin</code>, <code>postings.bin</code></div>

    <div class="label">Query:</div>
    <div class="row">
      <input class="q" type="text" name="q" value="{q}" placeholder="deep AND learning">
      <span>TopK:</span>
      <input class="topk" type="text" name="topk" value="{topk}">
      <input class="btn" type="submit" value="Search">
    </div>

    <div class="examples">
      <div><b>Examples:</b></div>
      <ul>
        {examples_html}
      </ul>
    </div>
  </form>

  <div class="meta">Binary: <code>{search_bin}</code><br>{bin_status}</div>

  <hr>

  {result_html}

</body>
</html>
"""

def make_results_html(docs):
    load_titles()
    if docs is None:
        return ""
    if len(docs) == 0:
        return '<div class="shown">Shown: 0</div><div>(no documents)</div>'

    lines = []
    lines.append(f'<div class="shown">Shown: {len(docs)}</div>')
    lines.append("<ol>")
    for doc_id in docs:
        title = TITLE_CACHE.get(doc_id, f"doc_{doc_id:06d}")
        path = str(TEXT_DIR / f"{doc_id:06d}.txt")
        lines.append(
            "<li>"
            f'<div class="line1">{html.escape(str(doc_id))} — {html.escape(title)}</div>'
            f'<div class="line2">{html.escape(path)}</div>'
            "</li>"
        )
    lines.append("</ol>")
    return "\n".join(lines)

@app.route("/", methods=["GET"])
def index():
    index_dir = request.args.get("index_dir", DEFAULT_INDEX_DIR).strip() or DEFAULT_INDEX_DIR
    q = request.args.get("q", "").strip()
    topk = request.args.get("topk", "100").strip()

    examples_html = "\n".join(f"<li>{html.escape(x)}</li>" for x in EXAMPLES)

    bin_status = "OK" if SEARCH_BIN.exists() else "NOT FOUND"
    result_html = '<div class="shown">Shown: 0</div>'

    if q:
        docs, err = run_search(index_dir, q, topk)
        if err:
            result_html = f'<div class="error"><b>Error:</b><br>{html.escape(err)}</div>'
        else:
            result_html = make_results_html(docs)

    return HTML_PAGE.format(
        index_dir=html.escape(index_dir),
        q=html.escape(q),
        topk=html.escape(topk),
        examples_html=examples_html,
        search_bin=html.escape(str(SEARCH_BIN)),
        bin_status=html.escape(bin_status),
        result_html=result_html,
    )