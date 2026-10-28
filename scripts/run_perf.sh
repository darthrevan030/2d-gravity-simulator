#!/bin/sh
#SBATCH --job-name=perf
#SBATCH --output=slurm-out/perf-%j.out
#SBATCH --error=slurm-out/perf-%j.err
#SBATCH --exclusive
#SBATCH --time=00:10:00
# Measures time and energy of grav-aos and grav-soa on compute nodes (needs the release build).
# Run on the frontend without sbatch. Jobs are capped at 10 minutes and at a few per user, so
# each point and implementation is its own job, and a background loop on the frontend submits
# them one at a time, then collects the results. Progress: slurm-out/perf-batch-<batch>.log
#
# Usage: scripts/run_perf.sh [--reps R] [--ref-only] [--tag LABEL] [--valgrind] [--after JOBID]
#   --reps R     repetitions per point (default 5; fewer if they don't fit in the time limit)
#   --ref-only   only the 1K/10K reference cases
#   --tag L      label stored in every row
#   --valgrind   run cachegrind and callgrind on the 1K input instead
#   --after ID   start only once job ID has succeeded (used by run_build_perf.sh)
#
# Output: perf/results/perf_<batch>.csv (raw per-rep rows in perf/results/raw_<batch>/)
# Columns: tag,impl,scenario,n,iterations,reps,time_s,time_rsd_pct,
#          energy_j,energy_rsd_pct,power_w,final_objects
#
# time = duration_time, energy = RAPL power/energy-pkg/, power = energy / time (all from perf stat)
# time_rsd_pct / energy_rsd_pct = standard error of the mean in % (what perf stat -r reports)
set -eu

LIMIT_S=570  # stay under the 10 min job limit

require_job() {
  [ -n "${SLURM_JOB_ID:-}" ] || { echo "ERROR: $1 is only for the Slurm job; run scripts/run_perf.sh." >&2; exit 1; }
}

# --point <root> <work> <raw csv> <impl> <scenario> <input> <n> <iterations> <reps> <tag>
point_job() {
  root="$1"; work="$2"; raw="$3"; impl="$4"; scen="$5"; input="$6"; n="$7"; it="$8"; reps="$9"; tag="${10}"
  bin="$root/out/build/default/$impl/Release/grav-$impl"
  [ -x "$bin" ] || { echo "ERROR: $bin not found; run scripts/build.sh first." >&2; exit 1; }
  trap 'rm -rf "$work" 2>/dev/null || true' EXIT
  head -n "$n" "$input" >"$work/input/input.txt"
  start="$(date +%s)"
  r=0
  while [ "$r" -lt "$reps" ]; do
    r=$((r + 1))
    t0="$(date +%s)"
    (cd "$work" && perf stat -a -x, -e duration_time,power/energy-pkg/ \
       -o "$work/perf.csv" "$bin" "$it" 1000 1000 0.25 >/dev/null)
    # perf -x, lines: value,unit,event,...  (duration_time in ns, energy in J)
    awk -F, -v tag="$tag" -v impl="$impl" -v s="$scen" -v n="$n" -v it="$it" -v r="$r" -v reps="$reps" \
        -v objs="$(awk 'NR==1 {print $4}' "$work/final_config.txt")" '
      $3 ~ /duration_time/ { t = $1 / 1e9 }
      $3 ~ /energy-pkg/    { e = $1 }
      END {
        if (t == "" || t == 0 || e == "") { print "perf output missing time/energy" > "/dev/stderr"; exit 1 }
        printf "%s,%s,%s,%d,%d,%.6f,%.4f,%s\n", tag, impl, s, n, it, t, e, objs
        printf "  %-10s %-4s n=%-6d it=%-5d rep %d/%d %9.3f s %10.2f J %8.2f W\n",
               s, impl, n, it, r, reps, t, e, e / t > "/dev/stderr"
      }' "$work/perf.csv" >>"$raw"
    now="$(date +%s)"
    if [ "$r" -lt "$reps" ] && [ $((now + now - t0 - start)) -gt "$LIMIT_S" ]; then
      echo "  stopping after $r reps: the next one would not fit in the time limit" >&2
      break
    fi
  done
}

# collect <raw dir> <out csv>: one row per point, averaged over its reps
collect() {
  raw="$1"; out="$2"
  echo "tag,impl,scenario,n,iterations,reps,time_s,time_rsd_pct,energy_j,energy_rsd_pct,power_w,final_objects" >"$out"
  cat "$raw"/*.csv 2>/dev/null | awk -F, '
    function sem_pct(sum, sq, c,   mean, var) {
      if (c < 2) return 0
      mean = sum / c; var = (sq - c * mean * mean) / (c - 1)
      if (var < 0) var = 0
      return sqrt(var / c) / mean * 100
    }
    {
      k = $1 "," $2 "," $3 "," $4 "," $5
      if (!(k in c)) order[++m] = k
      c[k]++; st[k] += $6; sst[k] += $6 * $6; se[k] += $7; sse[k] += $7 * $7; objs[k] = $8
    }
    END {
      for (i = 1; i <= m; i++) {
        k = order[i]; t = st[k] / c[k]; e = se[k] / c[k]
        printf "%s,%d,%.6f,%.2f,%.4f,%.2f,%.3f,%s\n", k, c[k], t, sem_pct(st[k], sst[k], c[k]),
               e, sem_pct(se[k], sse[k], c[k]), e / t, objs[k]
      }
    }' >>"$out"
  echo "== $(($(wc -l <"$out") - 1)) points written to $out"
}

# --valgrind-job <root> <work>: cachegrind + callgrind on the 1K input, 20 iterations
valgrind_job() {
  root="$1"; work="$2"
  command -v valgrind >/dev/null || { echo "ERROR: valgrind not available." >&2; exit 1; }
  trap 'rm -rf "$work" 2>/dev/null || true' EXIT
  head -n 1000 "$root/ftest/reference/1k/input.txt" >"$work/input/input.txt"
  for impl in aos soa; do
    bin="$root/out/build/default/$impl/Release/grav-$impl"
    for tool in cachegrind callgrind; do
      if [ "$tool" = cachegrind ]; then annotate=cg_annotate; else annotate=callgrind_annotate; fi
      out="$root/perf/results/${tool}_${impl}_${SLURM_JOB_ID}.out"
      (cd "$work" && valgrind --tool="$tool" --"$tool"-out-file="$out" \
         --log-file="${out%.out}.log" "$bin" 20 1000 1000 0.25 >/dev/null)
      "$annotate" "$out" >"${out%.out}.txt" || true
      echo "  $impl $tool -> $out"
    done
  done
}

case "${1:-}" in
  --point) require_job --point; shift; point_job "$@"; exit 0 ;;
  --valgrind-job) require_job --valgrind-job; shift; valgrind_job "$@"; exit 0 ;;
esac

# Frontend: make every directory here (the nodes can't), then submit the jobs one at a time.
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
if [ ! -f "$ROOT/scripts/run_perf.sh" ]; then
  echo "ERROR: cannot find the repo root; run scripts/run_perf.sh directly, without sbatch." >&2
  exit 1
fi
cd "$ROOT"

REPS=5
REF_ONLY=0
VALGRIND=0
TAG="run"
AFTER=""
while [ $# -gt 0 ]; do
  case "$1" in
    --reps) REPS="$2"; shift ;;
    --ref-only) REF_ONLY=1 ;;
    --valgrind) VALGRIND=1 ;;
    --tag) TAG="$2"; shift ;;
    --after) AFTER="$2"; shift ;;
    *) echo "unknown option $1" >&2; exit 2 ;;
  esac
  shift
done

INPUT_1K="$ROOT/ftest/reference/1k/input.txt"
INPUT_10K="$ROOT/ftest/reference/10k/input.txt"
for input in "$INPUT_1K" "$INPUT_10K"; do
  [ -f "$input" ] || { echo "ERROR: $input not found." >&2; exit 1; }
done

new_work() {
  w="$(mktemp -d "$ROOT/perf/work.XXXXXX")"
  mkdir "$w/input"
  echo "$w"
}

# wait_job <id>: block until the job has left the queue
wait_job() {
  while [ -n "$(squeue -h -j "$1" 2>/dev/null)" ]; do sleep 20; done
}

# submit_wait <label> <run_perf.sh args...>: submit (retrying while over the job limit), then wait
submit_wait() {
  label="$1"; shift
  tries=0
  while ! id="$(sbatch --parsable scripts/run_perf.sh "$@")"; do
    tries=$((tries + 1))
    [ "$tries" -lt 60 ] || { echo "$(date +%T) giving up on $label: sbatch kept failing"; return 1; }
    echo "$(date +%T) sbatch failed for $label, retrying in 60 s"
    sleep 60
  done
  echo "$(date +%T) job $id: $label"
  wait_job "$id"
}

# wait_build: wait for the --after job and stop if it did not succeed
wait_build() {
  [ -n "$AFTER" ] || return 0
  echo "$(date +%T) waiting for build job $AFTER"
  wait_job "$AFTER"
  state="$(sacct -n -X -j "$AFTER" -o State 2>/dev/null | tr -d ' ' | head -n 1)"
  [ -z "$state" ] || [ "$state" = COMPLETED ] || { echo "build job $AFTER ended $state; not measuring"; return 1; }
  for impl in aos soa; do
    [ -x "$ROOT/out/build/default/$impl/Release/grav-$impl" ] || { echo "grav-$impl missing after build"; return 1; }
  done
}

if [ -z "$AFTER" ]; then
  for impl in aos soa; do
    bin="$ROOT/out/build/default/$impl/Release/grav-$impl"
    [ -x "$bin" ] || { echo "ERROR: $bin not found; run scripts/build.sh first." >&2; exit 1; }
  done
fi

mkdir -p slurm-out perf/results
BATCH="$(date +%Y%m%d-%H%M%S)"
LOG="$ROOT/slurm-out/perf-batch-$BATCH.log"

if [ "$VALGRIND" -eq 1 ]; then
  WORK="$(new_work)"
  ( trap '' HUP; wait_build && submit_wait valgrind --valgrind-job "$ROOT" "$WORK" ) </dev/null >"$LOG" 2>&1 &
  echo "valgrind run started in the background; progress: $LOG"
  exit 0
fi

RAW="$ROOT/perf/results/raw_$BATCH"
OUT="$ROOT/perf/results/perf_$BATCH.csv"
mkdir "$RAW"
SEQ=0

# point <scenario> <input file> <n objects> <iterations>   (box 1000x1000, dt 0.25)
point() {
  for impl in aos soa; do
    SEQ=$((SEQ + 1))
    raw="$RAW/$(printf %02d "$SEQ")_${impl}_$1_$3_$4.csv"
    submit_wait "$impl $1 n=$3 it=$4" --point \
      "$ROOT" "$(new_work)" "$raw" "$impl" "$1" "$2" "$3" "$4" "$REPS" "$TAG" || true
  done
}

run_batch() {
  wait_build || return 1
  point reference "$INPUT_1K" 1000 500
  point reference "$INPUT_10K" 10000 100
  if [ "$REF_ONLY" -eq 0 ]; then
    for n in 500 1000 2000 4000 6000 8000 10000; do point scale_n "$INPUT_10K" "$n" 100; done
    for it in 100 200 500 1000 2000; do point scale_iter "$INPUT_1K" 1000 "$it"; done
  fi
  collect "$RAW" "$OUT"
}

( trap '' HUP; run_batch ) </dev/null >"$LOG" 2>&1 &
echo "perf batch $BATCH started in the background (one job at a time; safe to log out)"
echo "progress: tail -f $LOG"
echo "results:  $OUT"
