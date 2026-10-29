#!/usr/bin/env bash
# Builds one source tree per tagged stage into test-optimisations/stages/<tag>/, straight from git
# objects, so the working tree is never touched. Each stage is the tag exactly as committed (its own
# sources, presets and flags); see README.md for what each tag contains.
#
# Usage: test-optimisations/make_stages.sh [tag ...]   (default: baseline optimisation-1 optimisation-2)
# Env:   CXX (default g++-15)
# Needs the tags (run `git fetch --tags` first).
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/.." && pwd)"
STAGES="$HERE/stages"
DEPS="$REPO/out/build/default/_deps"  # reused if present, so no download is needed
ALL="baseline optimisation-1 optimisation-2"  # keep in sync with STAGES in run_bench.py
export CXX="${CXX:-g++-15}"
mkdir -p "$STAGES"

for name in ${*:-$ALL}; do
  git -C "$REPO" rev-parse -q --verify "refs/tags/$name^{commit}" >/dev/null \
    || { echo "ERROR: tag $name not found (run git fetch --tags)" >&2; exit 2; }
  d="$STAGES/$name"
  rm -rf "$d"; mkdir -p "$d"
  git -C "$REPO" archive "refs/tags/$name" | tar -x -C "$d"
  fetch=()
  [ -d "$DEPS/gsl-src" ] && fetch=(-DFETCHCONTENT_FULLY_DISCONNECTED=ON -DFETCHCONTENT_SOURCE_DIR_GSL="$DEPS/gsl-src"
                                   -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST="$DEPS/googletest-src")
  # -g only adds debug info (same code), so valgrind can annotate source lines
  if (cd "$d" && cmake --preset default -DCMAKE_CXX_FLAGS=-g ${fetch[@]+"${fetch[@]}"} >"$d/configure.log" 2>&1 \
        && cmake --build --preset release --target grav-aos grav-soa >"$d/build.log" 2>&1); then
    warnings=$(grep -c "warning:" "$d/build.log" || true)
    echo "built $name ($(git -C "$REPO" rev-parse --short "$name^{commit}"), warnings: $warnings)"
  else
    echo "FAILED $name (see $d/build.log)"
  fi
done
