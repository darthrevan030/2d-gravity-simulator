#!/usr/bin/env bash
# Build one unit test target with GCC, run it, and write its coverage reports.
#
# Usage: scripts/check_coverage.sh [common|aos|soa]     (default: common)
# Env:   CXX   GCC 15+ compiler (default: g++-15 if installed, else g++)
#        GCOV  matching gcov (default: gcov-NN for g++-NN, else gcov)
# Output: out/build/coverage/coverage-<lib>/coverage_report.{html,txt}
# Exit code: 0 if the build and every test pass, non-zero otherwise.
set -euo pipefail

LIB="${1:-common}"
case "$LIB" in
  common|aos|soa) ;;
  *) echo "usage: check_coverage.sh [common|aos|soa]" >&2; exit 2 ;;
esac
TARGET="ut$LIB"

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

# On macOS plain g++ is Apple clang, whose coverage files GCC's gcov cannot read
if [[ -z "${CXX:-}" ]]; then
  if command -v g++-15 >/dev/null; then CXX=g++-15; else CXX=g++; fi
fi
CXX="$(command -v "$CXX")"
if "$CXX" --version | grep -qi clang; then
  echo "error: $CXX is clang; set CXX to a GCC 15+ compiler" >&2; exit 1
fi
if (( "$("$CXX" -dumpversion | cut -d. -f1)" < 15 )); then
  echo "error: $CXX is GCC $("$CXX" -dumpversion); the code needs GCC 15+" >&2; exit 1
fi
CC="${CXX/g++/gcc}"
if [[ -z "${GCOV:-}" ]]; then
  suffix="${CXX##*g++}"   # "-15" for g++-15, "" for g++
  GCOV="gcov$suffix"
fi

GCOVR="$ROOT/.venv/bin/gcovr"
[[ -x "$GCOVR" ]] || GCOVR="$(command -v gcovr)" || { echo "error: gcovr not found" >&2; exit 1; }

# Its own build dir, so it never mixes with out/build/default (which may be built with clang)
BUILD="$ROOT/out/build/coverage"
REPORT="$BUILD/coverage-$LIB"

if [[ ! -f "$BUILD/CMakeCache.txt" ]]; then
  echo "== Configure ($CXX)"
  cmake --preset default -B "$BUILD" -DCMAKE_CXX_COMPILER="$CXX" -DCMAKE_C_COMPILER="$CC" >/dev/null
fi

echo "== Build $TARGET (Debug)"
cmake --build "$BUILD" --config Debug --target "$TARGET"

# Old counts would add up with this run's, and the build's test discovery also writes some
find "$BUILD" -name '*.gcda' -delete

echo "== Run $TARGET"
"$BUILD/$TARGET/Debug/$TARGET"

echo "== Coverage of $LIB/"
mkdir -p "$REPORT"
# Same filters as the coverage-* targets in cmake/TestUtils.cmake; the trailing $BUILD keeps
# gcovr from reading coverage files of other build dirs
gcovr_args=(--gcov-executable "$GCOV" --root "$ROOT" --filter "$LIB/" --exclude "out/"
            --exclude-unreachable-branches --exclude-throw-branches "$BUILD")
"$GCOVR" "${gcovr_args[@]}" --html --html-details -o "$REPORT/coverage_report.html" \
         --html-theme green --print-summary
"$GCOVR" "${gcovr_args[@]}" --txt -o "$REPORT/coverage_report.txt" --txt-metric branch
"$GCOVR" "${gcovr_args[@]}" --txt

echo "Branch misses per line: $REPORT/coverage_report.txt"
echo "Line-by-line report:    $REPORT/coverage_report.html"
