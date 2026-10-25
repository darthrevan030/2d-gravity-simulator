#!/usr/bin/env bash
# Functional tests for grav-aos and grav-soa (the real executables).
#
# Usage: ./ftest.sh
#
# Test groups
#   1. Error cases      wrong argument count / bad values / missing input
#                       -> exit code != 0, message on stderr prefixed "grav-xxx:"
#   2. Hand cases       ftest/cases/<name>/: small inputs, one rule each
#   3. Reference cases  ftest/reference/<name>/: instructor's inputs and outputs
#
# Each case folder holds: args, input.txt, expected/{init,final}_config.txt.
# Outputs are compared with tools/compare.py (values within 0.001).
# Every run happens in a fresh temp directory with its own input/input.txt,
# so the repository's input/ folder is never touched.
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

AOS="$ROOT/out/build/default/aos/Release/grav-aos"
SOA="$ROOT/out/build/default/soa/Release/grav-soa"
COMPARE="$ROOT/tools/compare.py"
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
echo "== 1. Error cases"
VALID_INPUT="$TMP/valid_input.txt"
echo "10.0 1.0 50.0 50.0 0.0 0.0" >"$VALID_INPUT"

# error_case <name> <args...>: both binaries must fail with a prefixed stderr message
error_case() {
  local name="$1"; shift
  local bin exe d
  for bin in "$AOS" "$SOA"; do
    exe="$(basename "$bin")"
    d="$(fresh_dir "err_${exe}_${name}" "$VALID_INPUT")"
    if run_in "$d" "$bin" "$@"; then
      fail "$exe $name: exit code 0, expected failure"
    elif ! grep -q "^$exe:" "$d/stderr.txt"; then
      fail "$exe $name: no stderr message prefixed with '$exe:'" "$(cat "$d/stderr.txt")"
    else
      ok "$exe $name"
    fi
  done
}

error_case "no_args"
error_case "3_args"            100 1000 500
error_case "5_args"            100 1000 500 0.25 7
error_case "iters_zero"        0 1000 500 0.25
error_case "iters_negative"    -5 1000 500 0.25
error_case "iters_not_number"  abc 1000 500 0.25
error_case "iters_not_integer" 1.5 1000 500 0.25
error_case "width_negative"    100 -1000 500 0.25
error_case "width_zero"        100 0 500 0.25
error_case "width_not_number"  100 abc 500 0.25
error_case "height_negative"   100 1000 -500 0.25
error_case "height_zero"       100 1000 0 0.25
error_case "dt_negative"       100 1000 500 -0.25
error_case "dt_zero"           100 1000 500 0
error_case "dt_not_number"     100 1000 500 xyz

# Team decision: missing input/input.txt is an error (not specified in the statement).
for bin in "$AOS" "$SOA"; do
  exe="$(basename "$bin")"
  d="$(fresh_dir "err_${exe}_no_input")"
  if ! run_in "$d" "$bin" 10 100 100 0.1 && [[ -s "$d/stderr.txt" ]]; then
    ok "$exe missing_input"
  else
    fail "$exe missing_input: expected non-zero exit + stderr message"
  fi
done

# ---------------------------------------------------------------------------
# run_case <case dir>: run both binaries on the case, compare with expected/
run_case() {
  local case_dir="$1" name bin exe d f msg start secs
  name="$(basename "$case_dir")"
  read -r -a args <"$case_dir/args"
  for bin in "$AOS" "$SOA"; do
    exe="$(basename "$bin")"
    d="$(fresh_dir "${exe}_${name}" "$case_dir/input.txt")"
    start=$(date +%s.%N)
    if ! run_in "$d" "$bin" "${args[@]}"; then
      fail "$exe $name: non-zero exit" "$(cat "$d/stderr.txt")"; continue
    fi
    secs=$(awk -v s="$start" -v e="$(date +%s.%N)" 'BEGIN { printf "%.2f", e - s }')
    for f in init_config.txt final_config.txt; do
      if [[ ! -f "$d/$f" ]]; then
        fail "$exe $name: $f not written"; continue 2
      fi
      if ! msg="$(python3 "$COMPARE" "$d/$f" "$case_dir/expected/$f" 2>&1)"; then
        fail "$exe $name: $f" "$msg"; continue 2
      fi
    done
    ok "$exe $name (${secs}s)"
  done
}

echo "== 2. Hand cases"
for case_dir in "$ROOT"/ftest/cases/*/; do
  run_case "$case_dir"
done

echo "== 3. Instructor reference cases"
run_case "$ROOT/ftest/reference/1k"

# ---------------------------------------------------------------------------
echo
echo "Functional tests: $PASS passed, $FAIL failed"
(( FAIL == 0 ))
