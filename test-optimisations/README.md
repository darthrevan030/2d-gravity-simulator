# test-optimisations

Measures what each optimisation did, in two ways:

- **Stages** (`make_stages.sh`, `run_bench.py`): the code before the optimisation pass (03e761e),
  with the changes added one at a time, so every change gets its own number.
- **Timeline** (`timeline.py`): every commit that touched the code, built exactly as it was,
  from the first version up to now.

`avignon.sh` runs both on the avignon compute nodes, measures energy as well as time, and runs
valgrind (cachegrind + callgrind) on every distinct executable.

Every run's output is checked against `ftest/reference/<case>/expected` with `tools/compare.py`
(values within 0.001). Cases: **1K** = 1000 objects, 500 iterations; **10K** = 10 000 objects,
100 iterations. Both use a 1000 × 1000 box and dt = 0.25.

## The stages

Each stage is 03e761e plus the changes listed, built with the project's warning flags (`-Werror`).

| Stage | Contents | Author |
| --- | --- | --- |
| `a_restart` | AOS collisions as before 7fe1cfd: erase + restart the scan after every merge | ergin04eda |
| `s0_base` | 03e761e: one-pass collisions with tombstones (AOS 7fe1cfd, SOA 32c9991) | ergin04eda, Emily Stringfellow |
| `s1_locals` | SOA: particle i's force sum in local variables instead of a reference into `ax` | darthrevan030 (3423ff7, part) |
| `s2_onediv` | + one division per pair instead of two | darthrevan030 (3423ff7, part) |
| `s3_iterators` | + vector iterators instead of `zip \| drop`, so g++ vectorizes the loop (= 3423ff7) | darthrevan030 |
| `a1_forces` | AOS: force sum in locals, divide by mass once per particle (4747e60) | darthrevan030 |
| `g_grid_only` | collision grid alone, on 03e761e (6d7ad57, 83b82fa, ea34a37, 4a2c944) | darthrevan030 |
| `x_grid` | all code changes: force loops + grid | darthrevan030 |
| `x_flags` | + `-fno-math-errno -fassociative-math -fno-signed-zeros -fno-trapping-math` (57af213) = final | darthrevan030 |
| `m_native` | `x_flags` + `-march=native` (not adopted) | |
| `m_fastmath` | `x_flags` with `-ffast-math` instead of the four flags (not adopted) | |
| `e1_alive` | AOS: `std::vector<char>` alive flags instead of tombstones (958d05d, eda_optimization) | ergin04eda |
| `e2_hoist` | AOS: `pi`/`pj` references hoisted in the force loop (3197d95, eda_optimization) | ergin04eda |
| `e3_simulate` | `x_flags` + enclosure and particle reference hoisted in `simulate` (72a418e) | ergin04eda |

`s1_locals` and `s2_onediv` don't exist as commits: 3423ff7 made three changes at once, so
`make_stages.sh` applies them one at a time to measure each separately. `e1_alive` uses 958d05d
and not the branch tip 765f7e6, because 765f7e6 removes a `static_cast` and no longer compiles
with `-Wsign-conversion -Werror`.

## Results on a Mac (Apple M5 Pro, g++-15, median of 5 runs, `results.csv`)

These are **not** avignon numbers: no energy, and ARM rather than x86 (NEON 128-bit vectors, no AVX).
The Mac's speed also drifts by 10–15% over tens of minutes (the same executable took 8.1 s at one
point and 9.2 s at another), so only numbers measured in the same session are comparable. These
stage results were measured twice and agreed to within 1%. The scripts now time round-robin
(run 1 of everything, then run 2, …) so drift affects every stage alike.

### SOA

| Stage | 1K | 10K | Step (10K) |
| --- | --- | --- | --- |
| `s0_base` | 0.555 s | 9.214 s | |
| `s1_locals` | 0.355 s | 5.654 s | −39% |
| `s2_onediv` | 0.337 s | 5.456 s | −4% |
| `s3_iterators` | 0.307 s | 4.940 s | −9% |
| `x_grid` | 0.109 s | 1.379 s | −72% |
| `x_flags` | 0.089 s | 1.045 s | −24% |
| total | 6.2× | 8.8× | |

### AOS

| Stage | 1K | 10K | Step (10K) |
| --- | --- | --- | --- |
| `a_restart` | 0.371 s | 41.051 s | |
| `s0_base` | 0.357 s | 4.497 s | −89% (9.1×) |
| `a1_forces` | 0.216 s | 3.357 s | −25% |
| `x_grid` | 0.146 s | 1.919 s | −43% |
| `x_flags` | 0.136 s | 1.720 s | −10% |
| total | 2.7× | 23.9× (2.6× from `s0_base`) | |

### Each change on its own

| Change | Compared with | AOS 10K | SOA 10K |
| --- | --- | --- | --- |
| collision grid | `g_grid_only` vs `s0_base` | 4.497 → 3.089 s | 9.214 → 5.702 s |
| `-march=native` | `m_native` vs `x_flags` | 1.719 s, identical executable | 1.034 s, identical executable |
| `-ffast-math` | `m_fastmath` vs `x_flags` | 1.741 vs 1.720 s | 1.033 vs 1.045 s |
| alive-flag vector | `e1_alive` vs `s0_base` | 4.485 vs 4.497 s (noise) | |
| hoisted references | `e2_hoist` vs `s0_base` | 4.489 vs 4.497 s (noise) | |
| hoisted enclosure | `e3_simulate` vs `x_flags` | 1.715 vs 1.720 s (noise) | |

What the numbers show:

- Before the pass, SOA was 2× *slower* than AOS (9.2 vs 4.5 s); after it, it is 1.6× faster
  (1.05 vs 1.72 s). The layout pays off only once the loop lets the compiler use it.
- The restart scan costs roughly merges × n², so it doesn't matter at 1K (0.371 vs 0.357 s) and
  dominates at 10K (41 vs 4.5 s).
- The collision check over all pairs was about 3.5 s of SOA's 9.2 s, and 1.4 s of AOS's 4.5 s.
- The reassociation flags help SOA more (−24% vs −10%) because only SOA's force loop is
  vectorized, and the flags let g++ reorder the sums inside the vector registers.
- The eda_optimization changes are correct but make no measurable difference: at `-O3`, g++
  already moves loop-invariant code out of the loop (loop-invariant code motion).
- `-march=native` changes nothing on this chip (g++ produced the same executable). On avignon
  (x86) it enables AVX, so measure it there.

## Running it

On a laptop:

```sh
test-optimisations/make_stages.sh                    # builds into test-optimisations/stages/ (CXX=g++-15)
test-optimisations/run_bench.py [--reps 5] [stage …] # appends to test-optimisations/results.csv
test-optimisations/timeline.py --branch origin/eda_optimization [--working-tree]
                                                     # writes test-optimisations/timeline.csv
```

On avignon, on the frontend (not with sbatch). This folder must be on the cluster: it is not
committed yet, so copy it there or commit it first.

```sh
git fetch origin
test-optimisations/avignon.sh [--reps 5] [--jobs 1] [--no-stages] [--no-timeline] [--working-tree] \
                              [--no-valgrind | --valgrind-only] [--vg-iterations 20]
tail -f test-optimisations/avignon-results/<batch>/progress.log
```

The frontend builds every stage and every commit and keeps one copy of each distinct executable
(compared without symbols, which include build paths), so code that didn't change is measured once. It then submits one 10-minute job per
executable and case, each running `perf stat -e duration_time,power/energy-pkg/`. When all jobs
are done it writes `avignon-results/<batch>/results.csv`, with time, energy and power (mean and
standard error) for every stage and commit. `--no-timeline` skips the history and its roughly 90
builds, if time is short.

Valgrind: one more job per distinct executable runs cachegrind (`--cache-sim=yes --branch-sim=yes`)
and callgrind on the 1K input for 20 iterations (`--vg-iterations`). Valgrind is 50–100× slower,
and its counts are exact and grow in proportion to the iterations, so a short run compares the
stages fairly. Outputs:

- `avignon-results/<batch>/valgrind.csv`: one row per stage/commit and implementation, with
  instructions (`I_refs`), data accesses (`D_refs`), L1 data misses and rate, last-level misses and
  rate, branches, mispredictions and rate;
- `avignon-results/<batch>/valgrind/<executable>/cachegrind.txt` and `callgrind.txt`: the full
  reports (`callgrind.txt` lists the instructions spent in each function). `<executable>` is the
  `binary_id` column of the CSVs.

The release builds have symbols but no debug info, so callgrind gives per-function counts. For
per-line detail, rebuild that stage with `-g` and run valgrind by hand.
