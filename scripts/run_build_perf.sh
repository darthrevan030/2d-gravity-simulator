#!/bin/sh
# Submits the release build, then the perf jobs once the build succeeds. Run on the frontend.
#
# Usage: scripts/run_build_perf.sh [run_perf.sh options...]
#   e.g. scripts/run_build_perf.sh --reps 3 --tag baseline
set -eu

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
mkdir -p slurm-out

build_id="$(sbatch --parsable scripts/build.sh all release)"
echo "build job $build_id"
scripts/run_perf.sh --after "$build_id" "$@"
