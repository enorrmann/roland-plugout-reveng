#!/bin/sh
# Runs web/test/page-test.html in headless Chrome against a local server of the repo
# (docs/WEB_INSTALLER.md §5). No real MIDI port is opened: the page gets a fake Web MIDI.
#   sh web/test/run-page-test.sh [/path/from/repo/root/Code1.Dat]
# Exit code 0 = PASS. CHROME=... picks the browser (default: google-chrome, chromium).
set -e
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
DAT=${1:-/plugins/C-S1SY13F/Script/Code1.Dat}
CHROME=${CHROME:-$(command -v google-chrome || command -v chromium || command -v chromium-browser)}
[ -n "$CHROME" ] || { echo "no Chrome / Chromium found (set CHROME=...)" >&2; exit 2; }
PORT=${PORT:-8765}
TMP=$(mktemp -d)
python3 -m http.server "$PORT" --bind 127.0.0.1 --directory "$ROOT" >"$TMP/server.log" 2>&1 &
SERVER=$!
trap 'kill $SERVER 2>/dev/null; rm -rf "$TMP"' EXIT INT TERM
for _ in 1 2 3 4 5 6 7 8 9 10; do
    python3 -c "import urllib.request; urllib.request.urlopen('http://127.0.0.1:$PORT/web/')" 2>/dev/null && break
    sleep 0.3
done
"$CHROME" --headless=new --disable-gpu --no-first-run --user-data-dir="$TMP/profile" \
    --virtual-time-budget=36000000 --dump-dom \
    "http://127.0.0.1:$PORT/web/test/page-test.html?dat=$DAT" 2>/dev/null >"$TMP/dom.html"
python3 - "$TMP/dom.html" <<'PY'
import html, re, sys
m = re.search(r'<pre id="result">(.*?)</pre>', open(sys.argv[1], encoding='utf-8').read(), re.S)
out = html.unescape(m.group(1)) if m else 'FAIL no result in the page'
print(out)
sys.exit(0 if out.startswith('PASS') else 1)
PY
