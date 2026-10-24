#!/bin/sh
# Submit the release build, then the perf job once the build succeeds.
#
# Usage: scripts/run_build_perf.sh [run_perf.sh options...]
#   e.g. scripts/run_build_perf.sh --reps 3 --tag baseline
#        scripts/run_build_perf.sh --valgrind
set -eu

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"
mkdir -p slurm-out

build_id="$(sbatch --parsable scripts/build.sh all release)"
perf_id="$(sbatch --parsable --dependency=afterok:"$build_id" scripts/run_perf.sh "$@")"
echo "build job $build_id -> perf job $perf_id (logs in slurm-out/)"
