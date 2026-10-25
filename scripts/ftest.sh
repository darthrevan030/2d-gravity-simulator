#!/usr/bin/env bash
# Functional tests for grav-aos and grav-soa (the real executables).
#
# Usage: ./ftest.sh
# Every run happens in a fresh temp directory with its own input/input.txt,
# so the repository's input/ folder is never touched.
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

AOS="$ROOT/out/build/default/aos/Release/grav-aos"
SOA="$ROOT/out/build/default/soa/Release/grav-soa"
for b in "$AOS" "$SOA"; do
  [[ -x "$b" ]] || { echo "missing binary: $b" >&2; exit 1; }
done

PASS=0
FAIL=0
TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT

ok()   { PASS=$((PASS + 1)); printf '  PASS %s\n' "$1"; }
fail() { FAIL=$((FAIL + 1)); printf '  FAIL %s\n' "$1"; [[ -n "${2:-}" ]] && sed 's/^/       /' <<<"$2"; }

# fresh_dir <name> [input file]: new working dir with optional input/input.txt
fresh_dir() {
  local d="$TMP/$1"
  rm -rf "$d"; mkdir -p "$d/input"
  [[ -n "${2:-}" ]] && cp "$2" "$d/input/input.txt"
  echo "$d"
}

# run_in <dir> <binary> <args...>: run binary inside dir, stdout/stderr to files
run_in() {
  local dir="$1" bin="$2"; shift 2
  (cd "$dir" && "$bin" "$@" >stdout.txt 2>stderr.txt)
}

# ---------------------------------------------------------------------------
echo
echo "Functional tests: $PASS passed, $FAIL failed"
(( FAIL == 0 ))
