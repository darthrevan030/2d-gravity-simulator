#!/bin/sh
#SBATCH --job-name=build
#SBATCH --output=slurm-out/build-%j.out
#SBATCH --error=slurm-out/build-%j.err
# Configure and build the project.

# Usage: sbatch scripts/build.sh [aos|soa|all] [release|debug]   (submit from the repo root)
#        scripts/build.sh [aos|soa|all] [release|debug]          (run directly)
# Defaults: all, release. The slurm-out/ directory must exist before sbatch is called.
set -eu

IMPL="${1:-all}"
CONFIG="${2:-release}"
case "$IMPL" in
  aos) TARGETS="grav-aos" ;;
  soa) TARGETS="grav-soa" ;;
  all) TARGETS="grav-aos grav-soa" ;;
  *) echo "usage: build.sh [aos|soa|all] [release|debug]" >&2; exit 2 ;;
esac
case "$CONFIG" in
  release|debug) ;;
  *) echo "usage: build.sh [aos|soa|all] [release|debug]" >&2; exit 2 ;;
esac

ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"
cd "$ROOT"

# -g only adds debug info (g++ generates the same code), so valgrind reports can be annotated line
# by line; the graded flags in CMakePresets.json are untouched
cmake --preset default -DCMAKE_CXX_FLAGS=-g
# shellcheck disable=SC2086  # TARGETS is intentionally word-split
cmake --build --preset "$CONFIG" --target $TARGETS
echo "Finished building $IMPL ($CONFIG)"
