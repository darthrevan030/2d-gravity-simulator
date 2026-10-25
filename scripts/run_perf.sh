#!/bin/sh
#SBATCH --job-name=perf
#SBATCH --output=slurm-out/perf-%j.out
#SBATCH --error=slurm-out/perf-%j.err
#SBATCH --exclusive
# Performance and energy measurements. Expects the release build to exist
# (scripts/build.sh); scripts/run_build_perf.sh submits both in order.
# Must be submitted with sbatch from the repo root: the measurements have to run
# on an exclusive compute node, not on the frontend.

# Usage: sbatch scripts/run_perf.sh [--reps R] [--tag LABEL]
#   --reps R     perf stat repetitions per point (default 5)
#   --tag L      label stored in every row (e.g. "baseline", "O3-lto")

# Scenarios (all inputs come from the prof's files):
#   reference   1K (500 it) and 10K (100 it), the provided cases

# Output: perf/results/perf_<jobid>.csv
# Columns: tag,impl,scenario,n,iterations,reps,time_s,time_rsd_pct,
#          energy_j,energy_rsd_pct,power_w,final_objects

# time = duration_time,
# energy = RAPL power/energy-pkg/,
# power = energy / time.
set -eu

# Refuse to run outside a Slurm job (e.g. directly on the frontend node).
if [ -z "${SLURM_JOB_ID:-}" ]; then
  echo "ERROR: run this with sbatch (scripts/run_build_perf.sh or sbatch scripts/run_perf.sh)." >&2
  exit 1
fi

# Slurm runs a spooled copy of this script, so the repo root is where sbatch was called.
ROOT="$SLURM_SUBMIT_DIR"
REPS=5
TAG="run"
while [ $# -gt 0 ]; do
  case "$1" in
    --reps) REPS="$2"; shift ;;
    --tag) TAG="$2"; shift ;;
    *) echo "unknown option $1" >&2; exit 2 ;;
  esac
  shift
done

# Check that the required binaries and input files exist.
AOS="$ROOT/out/build/default/aos/Release/grav-aos"
SOA="$ROOT/out/build/default/soa/Release/grav-soa"
INPUT_1K="$ROOT/ftest/reference/1k/input.txt"
INPUT_10K="$ROOT/ftest/reference/10k/input.txt"

# Check that the binaries exist and are executable.
for bin in "$AOS" "$SOA"; do
  [ -x "$bin" ] || { echo "ERROR: $bin not found; run scripts/build.sh first." >&2; exit 1; }
done

# Check that the input files exist.
for input in "$INPUT_1K" "$INPUT_10K"; do
  [ -f "$input" ] || { echo "ERROR: $input not found." >&2; exit 1; }
done

# The Slurm job id (assigned by the scheduler) makes the output file names unique
# and matches slurm-out/perf-<id>.out.
JOB="$SLURM_JOB_ID"
mkdir -p "$ROOT/perf/results"

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK/input"

OUT="$ROOT/perf/results/perf_${JOB}.csv"
echo "tag,impl,scenario,n,iterations,reps,time_s,time_rsd_pct,energy_j,energy_rsd_pct,power_w,final_objects" >"$OUT"

# measure <scenario> <input file> <n objects> <iterations>   (box 1000x1000, dt 0.25)
measure() {
  scen="$1"; input="$2"; n="$3"; it="$4"
  head -n "$n" "$input" >"$WORK/input/input.txt"
  for impl in aos soa; do
    if [ "$impl" = aos ]; then bin="$AOS"; else bin="$SOA"; fi
    (cd "$WORK" && perf stat -a -x, -r "$REPS" -e duration_time,power/energy-pkg/ \
       -o "$WORK/perf.csv" "$bin" "$it" 1000 1000 0.25 >/dev/null)
    # perf -x, lines: value,unit,event,variance%,...  (duration_time in ns, energy in J)
    awk -F, -v tag="$TAG" -v impl="$impl" -v s="$scen" -v n="$n" -v it="$it" \
        -v r="$REPS" -v objs="$(awk 'NR==1 {print $4}' "$WORK/final_config.txt")" '
      $3 ~ /duration_time/ { t = $1 / 1e9; trsd = $4 }
      $3 ~ /energy-pkg/    { e = $1;       ersd = $4 }
      END {
        if (t == "" || t == 0 || e == "") { print "perf output missing time/energy" > "/dev/stderr"; exit 1 }
        gsub(/[%+-]/, "", trsd); gsub(/[%+-]/, "", ersd)
        printf "%s,%s,%s,%d,%d,%d,%.6f,%s,%.4f,%s,%.3f,%s\n",
               tag, impl, s, n, it, r, t, trsd, e, ersd, e / t, objs
        printf "  %-10s %-4s n=%-6d it=%-5d %9.3f s %10.2f J %8.2f W\n",
               s, impl, n, it, t, e, e / t > "/dev/stderr"
      }' "$WORK/perf.csv" >>"$OUT"
  done
}

echo "== Writing $OUT"
echo "-- reference (pass: 1K < 3 s, 10K < 50 s | goal ~1.2 s / ~20 s)"
measure reference "$INPUT_1K"  1000  500
measure reference "$INPUT_10K" 10000 100
echo "== Done: $OUT"
