#!/usr/bin/env bash
#SBATCH --job-name=opt-stage
#SBATCH --exclusive
#SBATCH --time=00:10:00
# Measures time, energy and valgrind counts (instructions, cache misses, branch mispredictions) of
# the three tagged stages (baseline, optimisation-1, optimisation-2) on the avignon compute nodes,
# and optionally of every commit.
# Run on the frontend, without sbatch:
#
#   git fetch --tags origin                 # needs the tags (and origin/eda_optimization for --timeline)
#   test-optimisations/avignon.sh [--reps N] [--jobs K] [--no-stages] [--timeline] [--working-tree]
#                                 [--no-valgrind] [--valgrind-only] [--vg-iterations N]
#                                 [--vg-cache NAME:I1:D1:LL ...]
#     --reps N        runs per executable and case (default 5; fewer if they don't fit in 10 minutes)
#     --jobs K        jobs in the queue at the same time (default 1; each job has a whole node)
#     --no-stages     skip the tagged stages (make_stages.sh)
#     --timeline      also measure the per-commit history (timeline.py, roughly 90 builds)
#     --working-tree  also measure the uncommitted working tree in the history (implies --timeline)
#     --no-valgrind   skip the valgrind jobs
#     --valgrind-only only the valgrind jobs, no timing
#     --vg-iterations iterations for the valgrind runs on the 1K input (default 20; valgrind is
#                     50-100x slower, and the counts grow in proportion to the iterations)
#     --vg-cache C    an extra simulated cache for cachegrind, as NAME:I1:D1:LL with each cache
#                     size,associativity,line_size (bytes), e.g. the cache lab's
#                     --vg-cache lab:16384,2,32:16384,2,32:131072,2,32
#                     Repeat it to compare sizes and associativities. Always also run: "host",
#                     valgrind's model of the node's real caches.
#
# 1. On the frontend: builds every tag (and every commit) with $CXX (default g++-15), keeps one
#    copy of each distinct executable, and prepares a work folder for each run (compute
#    nodes cannot create folders).
# 2. Submits one job per executable and case (1K: 500 iterations, 10K: 100 iterations). Each job
#    runs `perf stat -e duration_time,power/energy-pkg/` --reps times, stopping early if the next
#    run would not fit in the 10-minute limit.
# 3. When all jobs are done: checks every output against ftest/reference/<case>/expected and writes
#    test-optimisations/avignon-results/<batch>/results.csv, one row per stage/commit, impl and case:
#    kind,label,order,commit,date,author,subject,impl,case,status,reps,time_s,time_sem_pct,
#    energy_j,energy_sem_pct,power_w,final_objects,binary_id,same_binary_as
#    (time and energy are means over the runs; *_sem_pct is the standard error of the mean in %).
#    Valgrind: one job per executable runs cachegrind (--cache-sim=yes --branch-sim=yes) once per
#    cache configuration, then callgrind, on the 1K input. Builds have -g (debug info only, same
#    code), so the reports are annotated line by line (--auto=yes), like in the cache lab.
#    Counts: avignon-results/<batch>/valgrind.csv, one row per stage/commit, impl and cache:
#    ...,cache,I1,D1,LL,I_refs,D_refs,D1_misses,D1_miss_rate_pct,LLd_misses,LL_miss_rate_pct,
#    branches,mispredicts,mispred_rate_pct,...
#    Reports: valgrind/<executable>/cachegrind-<cache>.txt and callgrind.txt.
#
# Steps 2 and 3 run in the background on the frontend and survive logging out.
# Progress: tail -f test-optimisations/avignon-results/<batch>/progress.log
set -eu

LIMIT_S=560  # stay under the 10-minute job limit

# --job <binary> <work dir> <raw csv> <case> <reps>: runs on a compute node
if [ "${1:-}" = "--job" ]; then
  [ -n "${SLURM_JOB_ID:-}" ] || { echo "ERROR: --job is only for the Slurm job" >&2; exit 1; }
  bin="$2"; work="$3"; raw="$4"; case_name="$5"; reps="$6"
  id="$(basename "$bin")"
  args="$(cat "$work/args")"
  # one untimed warm-up run for the short case, whose first run is slowed down by a cold start
  # shellcheck disable=SC2086
  [ "$case_name" != 1k ] || (cd "$work" && "$bin" $args >/dev/null 2>&1) || true
  start="$(date +%s)"
  last=0
  r=0
  while [ "$r" -lt "$reps" ]; do
    r=$((r + 1))
    now="$(date +%s)"
    left=$((LIMIT_S - (now - start)))
    if [ "$r" -gt 1 ] && [ "$last" -ge "$left" ]; then
      echo "stopping after $((r - 1)) runs: the next one would not fit in the time limit" >&2
      break
    fi
    t0="$(date +%s)"
    status=0
    # shellcheck disable=SC2086  # args is intentionally word-split
    (cd "$work" && timeout -k 5 "$left" perf stat -a -x, -e duration_time,power/energy-pkg/ \
        -o "$work/perf.csv" "$bin" $args >/dev/null 2>"$work/stderr.txt") || status=$?
    last=$(($(date +%s) - t0))
    if [ "$status" -eq 124 ]; then echo "$id,$case_name,$r,timeout,," >>"$raw"; break; fi
    if [ "$status" -ne 0 ]; then echo "$id,$case_name,$r,crash,," >>"$raw"; break; fi
    awk -F, -v id="$id" -v c="$case_name" -v r="$r" '
      $3 ~ /duration_time/ { t = $1 / 1e9 }
      $3 ~ /energy-pkg/    { e = $1 }
      END {
        if (t == "" || e == "") { printf "%s,%s,%d,perf-error,,\n", id, c, r; exit }
        printf "%s,%s,%d,ok,%.6f,%.4f\n", id, c, r, t, e
      }' "$work/perf.csv" >>"$raw"
  done
  exit 0
fi

# --vg-job <binary> <work dir> <output dir> <caches file>: runs on a compute node
# caches file: one "<name> <I1> <D1> <LL>" per line ("-" = valgrind's model of this machine)
if [ "${1:-}" = "--vg-job" ]; then
  [ -n "${SLURM_JOB_ID:-}" ] || { echo "ERROR: --vg-job is only for the Slurm job" >&2; exit 1; }
  bin="$2"; work="$3"; vg="$4"; caches="$5"
  args="$(cat "$work/args")"
  if ! command -v valgrind >/dev/null; then echo no-valgrind >"$vg/status"; exit 0; fi
  start="$(date +%s)"
  # run <tool> <out name> <options...>: one valgrind run, then its annotated report
  run_tool() {
    tool="$1"; name="$2"; shift 2
    left=$((LIMIT_S - ($(date +%s) - start)))
    status=0
    # shellcheck disable=SC2086  # args is intentionally word-split
    (cd "$work" && timeout -k 5 "$left" valgrind --tool="$tool" "$@" --"$tool"-out-file="$vg/$name.out" \
        --log-file="$vg/$name.log" "$bin" $args >/dev/null 2>&1) || status=$?
    if [ "$status" -eq 124 ]; then echo "timeout ($name)" >"$vg/status"; exit 0; fi
    if [ "$status" -ne 0 ]; then echo "crash ($name)" >"$vg/status"; exit 0; fi
    if [ "$tool" = cachegrind ]; then annotate=cg_annotate; else annotate=callgrind_annotate; fi
    "$annotate" --auto=yes "$vg/$name.out" >"$vg/$name.txt" 2>&1 || true
  }
  while read -r name i1 d1 ll; do
    if [ "$i1" = - ]; then
      run_tool cachegrind "cachegrind-$name" --cache-sim=yes --branch-sim=yes
    else
      run_tool cachegrind "cachegrind-$name" --cache-sim=yes --branch-sim=yes --I1="$i1" --D1="$d1" --LL="$ll"
    fi
  done <"$caches"
  run_tool callgrind callgrind
  echo ok >"$vg/status"
  exit 0
fi

# ---------- frontend ----------
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/.." && pwd)"
cd "$ROOT"
command -v sbatch >/dev/null || { echo "ERROR: run this on the avignon frontend (no sbatch here)." >&2; exit 1; }

REPS=5
JOBS=1
STAGES=1
TIMELINE=0
VALGRIND=1
TIMING=1
VG_ITERATIONS=20
VG_CACHES="host - - -"
TIMELINE_ARGS=()
while [ $# -gt 0 ]; do
  case "$1" in
    --reps) REPS="$2"; shift ;;
    --jobs) JOBS="$2"; shift ;;
    --no-stages) STAGES=0 ;;
    --timeline) TIMELINE=1 ;;
    --working-tree) TIMELINE=1; TIMELINE_ARGS+=(--working-tree) ;;
    --no-valgrind) VALGRIND=0 ;;
    --valgrind-only) TIMING=0 ;;
    --vg-iterations) VG_ITERATIONS="$2"; shift ;;
    --vg-cache)
      IFS=: read -r name i1 d1 ll <<<"$2"
      for c in "$i1" "$d1" "$ll"; do
        echo "$c" | grep -Eq '^[0-9]+,[0-9]+,[0-9]+$' \
          || { echo "bad --vg-cache $2: expected NAME:size,assoc,line:size,assoc,line:size,assoc,line" >&2; exit 2; }
      done
      VG_CACHES="$VG_CACHES
$name $i1 $d1 $ll"
      shift ;;
    *) echo "unknown option $1" >&2; exit 2 ;;
  esac
  shift
done

export CXX="${CXX:-g++-15}"
command -v "$CXX" >/dev/null || { echo "ERROR: compiler $CXX not found (set CXX)." >&2; exit 1; }
if [ "$STAGES" -eq 1 ]; then
  for t in baseline optimisation-1 optimisation-2; do
    git rev-parse -q --verify "refs/tags/$t^{commit}" >/dev/null \
      || { echo "ERROR: tag $t missing; run git fetch --tags origin." >&2; exit 1; }
  done
fi
if [ "$TIMELINE" -eq 1 ]; then
  if git rev-parse -q --verify origin/eda_optimization >/dev/null; then
    TIMELINE_ARGS+=(--branch origin/eda_optimization)
  else
    echo "warning: origin/eda_optimization not found; its commits are left out of the history" >&2
  fi
fi

BATCH="$(date +%Y%m%d-%H%M%S)"
OUT="$HERE/avignon-results/$BATCH"
mkdir -p "$OUT/bin" "$OUT/raw" "$OUT/work" "$OUT/slurm" "$OUT/valgrind"
[ "$TIMING" -eq 1 ] || [ "$VALGRIND" -eq 1 ] || { echo "nothing to do: --valgrind-only with --no-valgrind" >&2; exit 2; }
echo "$VG_ITERATIONS" >"$OUT/vg-iterations.txt"
echo "$VG_CACHES" >"$OUT/vg-caches.txt"
echo "== batch $BATCH: $OUT"
{ hostname; "$CXX" --version | head -n 1; git rev-parse HEAD; } >"$OUT/build-info.txt"

# 1. build
if [ "$STAGES" -eq 1 ]; then
  echo "== building the tagged stages"
  "$HERE/make_stages.sh" | tee "$OUT/build-stages.log"
  python3 "$HERE/avignon_helper.py" stages-manifest --bin-dir "$OUT/bin" --out "$OUT/stages-manifest.csv"
fi
if [ "$TIMELINE" -eq 1 ]; then
  echo "== building every commit (this takes a while)"
  python3 "$HERE/timeline.py" --build-only "$OUT/timeline-manifest.csv" --bin-dir "$OUT/bin" \
    ${TIMELINE_ARGS[@]+"${TIMELINE_ARGS[@]}"} | tee "$OUT/build-timeline.log"
fi

manifests=()
[ -f "$OUT/stages-manifest.csv" ] && manifests+=("$OUT/stages-manifest.csv")
[ -f "$OUT/timeline-manifest.csv" ] && manifests+=("$OUT/timeline-manifest.csv")
[ "${#manifests[@]}" -gt 0 ] || { echo "nothing to measure" >&2; exit 1; }

# work folders, one per job (made here: the compute nodes cannot create folders)
# runs.txt: one job per line, "<perf|valgrind> <binary> <work dir> <case or output dir>"
python3 "$HERE/avignon_helper.py" unique-binaries "${manifests[@]}" >"$OUT/binaries.txt"
: >"$OUT/runs.txt"
while read -r bin impl; do
  id="$(basename "$bin")"
  if [ "$TIMING" -eq 1 ]; then
    for case_name in 1k 10k; do
      work="$OUT/work/$id-$case_name"
      mkdir -p "$work/input"
      cp "ftest/reference/$case_name/input.txt" "$work/input/input.txt"
      cp "ftest/reference/$case_name/args" "$work/args"
      echo "perf $bin $work $case_name" >>"$OUT/runs.txt"
    done
  fi
  if [ "$VALGRIND" -eq 1 ]; then
    work="$OUT/work/$id-valgrind"
    mkdir -p "$work/input" "$OUT/valgrind/$id"
    cp "ftest/reference/1k/input.txt" "$work/input/input.txt"
    echo "$VG_ITERATIONS 1000 1000 0.25" >"$work/args"
    echo "valgrind $bin $work $OUT/valgrind/$id" >>"$OUT/runs.txt"
  fi
done <"$OUT/binaries.txt"
echo "== $(wc -l <"$OUT/binaries.txt") distinct executables, $(wc -l <"$OUT/runs.txt") jobs to run"

# 2 + 3. submit the jobs (at most $JOBS in the queue), then collect
running_jobs() { squeue -h -u "$USER" -n opt-stage 2>/dev/null | wc -l; }
submit() {  # submit <args...>, retrying while the queue limit is reached
  local tries=0 id
  while ! id="$(sbatch --parsable -o "$OUT/slurm/%j.out" -e "$OUT/slurm/%j.err" "$HERE/avignon.sh" "$@")"; do
    tries=$((tries + 1))
    [ "$tries" -lt 60 ] || { echo "$(date +%T) giving up: sbatch kept failing"; return 1; }
    sleep 60
  done
  echo "$id"
}
run_all() {
  local n=0 total kind bin work last id
  total="$(wc -l <"$OUT/runs.txt")"
  while read -r kind bin work last; do
    while [ "$(running_jobs)" -ge "$JOBS" ]; do sleep 15; done
    n=$((n + 1))
    if [ "$kind" = perf ]; then
      id="$(submit --job "$bin" "$work" "$OUT/raw/$(basename "$work").csv" "$last" "$REPS")" || continue
    else
      id="$(submit --vg-job "$bin" "$work" "$last" "$OUT/vg-caches.txt")" || continue
    fi
    echo "$(date +%T) [$n/$total] job $id: $(basename "$work")"
  done <"$OUT/runs.txt"
  while [ "$(running_jobs)" -gt 0 ]; do sleep 15; done
  python3 "$HERE/avignon_helper.py" collect --batch "$OUT"
  echo "$(date +%T) done"
}
( trap '' HUP; run_all ) </dev/null >"$OUT/progress.log" 2>&1 &
echo "== jobs are being submitted in the background (safe to log out)"
echo "progress: tail -f $OUT/progress.log"
echo "results:  $OUT/results.csv and $OUT/valgrind.csv"
