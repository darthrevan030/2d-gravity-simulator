# test-optimisations

Measures what each optimisation pass did, using three git tags:

| Tag | Commit | Contents |
| --- | --- | --- |
| `baseline` | 8c8729e | first complete version. AOS collisions erase the merged particle and restart the scan after every merge |
| `optimisation-1` | 03e761e | AOS collisions in one pass, with tombstones (7fe1cfd, ergin04eda) |
| `optimisation-2` | 57af213 | force loops rewritten (AOS 4747e60, SOA 3423ff7), collision grid (6d7ad57, 83b82fa, ea34a37, 4a2c944, 810475d), and `-fno-math-errno -fassociative-math -fno-signed-zeros -fno-trapping-math` (57af213), all by darthrevan030 |

Each tag is built exactly as it was committed (its own sources, presets and flags), with `-g`
added for valgrind (debug info only: g++ generates the same code).

- **Stages** (`make_stages.sh`, `run_bench.py`): build and time the three tags.
- **Timeline** (`timeline.py`, optional): every commit that touched the code, built exactly as it
  was, from the first version up to now.

`avignon.sh` runs on the avignon compute nodes. It measures energy as well as time, and runs
valgrind (cachegrind + callgrind) on every distinct executable.

Every run's output is checked against `ftest/reference/<case>/expected` with `tools/compare.py`
(values within 0.001). Cases: **1K** = 1000 objects, 500 iterations; **10K** = 10 000 objects,
100 iterations. Both use a 1000 × 1000 box and dt = 0.25.

`baseline`'s AOS executable is reported as `wrong-output`. Its values are correct, but it writes
particle lines without the `<index>:` prefix (fixed in `config_writer.cpp` before
`optimisation-1`), so `compare.py` rejects the format. Its times are still valid.

The tags can be moved, so every CSV also records the commit each tag pointed to when it was
measured.

## Results on a Mac (Apple M5 Pro, g++-15, median of 5 runs, `results.csv`)

These are **not** avignon numbers: no energy, and ARM rather than x86 (NEON 128-bit vectors, no AVX).
The Mac's speed drifts by 10–15% over tens of minutes, so only numbers from the same session can be
compared. `run_bench.py` times round-robin (run 1 of every tag, then run 2, …) so the drift affects
every tag alike.

| Tag | AOS 1K | AOS 10K | SOA 1K | SOA 10K |
| --- | --- | --- | --- | --- |
| `baseline` | 0.359 s | 40.704 s | 0.548 s | 9.103 s |
| `optimisation-1` | 0.351 s | 4.419 s (9.2×) | 0.548 s | 9.079 s (same code) |
| `optimisation-2` | 0.134 s (2.6×) | 1.709 s (2.6×) | 0.087 s (6.3×) | 1.026 s (8.8×) |
| total | 2.7× | 23.8× | 6.3× | 8.9× |

What the numbers show:

- `optimisation-1` only changes AOS. The restart scan costs roughly merges × n², so it barely
  matters at 1K (0.359 vs 0.351 s) and dominates at 10K (40.7 vs 4.4 s).
- After `optimisation-1`, SOA is 2× *slower* than AOS at 10K (9.1 vs 4.4 s). After
  `optimisation-2` it is 1.7× faster (1.03 vs 1.71 s): the layout only pays off once the force loop
  is written so g++ can vectorize it.

## Running it

On a laptop:

```sh
test-optimisations/make_stages.sh [tag …]            # builds into test-optimisations/stages/<tag>/ (CXX=g++-15)
test-optimisations/run_bench.py [--reps 5] [tag …]   # appends to test-optimisations/results.csv
test-optimisations/timeline.py --branch origin/eda_optimization [--working-tree]
                                                     # writes test-optimisations/timeline.csv
```

On avignon, on the frontend (not with sbatch), in a checkout that contains this folder:

```sh
git fetch --tags origin
test-optimisations/avignon.sh [--reps 5] [--jobs 1] [--no-stages] [--timeline] [--working-tree] \
                              [--no-valgrind | --valgrind-only] [--vg-iterations 20]
tail -f test-optimisations/avignon-results/<batch>/progress.log
```

The frontend builds the three tags (and, with `--timeline`, every commit) and keeps one copy of each distinct executable
(compared without symbols, which include build paths), so code that didn't change is measured once. It then submits one 10-minute job per
executable and case, each running `perf stat -e duration_time,power/energy-pkg/`. When all jobs
are done it writes `avignon-results/<batch>/results.csv`, with time, energy and power (mean and
standard error) for every tag and commit. The history is off by default because it means roughly
90 more builds; `--timeline` (or `--working-tree`) turns it on.

Valgrind: one more job per distinct executable runs cachegrind (`--cache-sim=yes --branch-sim=yes`)
once per cache configuration, then callgrind, on the 1K input for 20 iterations (`--vg-iterations`).
Valgrind is 50–100× slower, and its counts are exact and grow in proportion to the iterations, so a
short run compares the stages fairly.

Cache configurations: `host` (valgrind's model of the node's real caches) is always run. Add
simulated ones with `--vg-cache NAME:I1:D1:LL`, each cache given as `size,associativity,line_size`
in bytes, as in the cache lab. Repeat the option to vary size and associativity:

```sh
test-optimisations/avignon.sh --valgrind-only \
  --vg-cache lab:16384,2,32:16384,2,32:131072,2,32 \
  --vg-cache d1-32k-8way:16384,2,32:32768,8,64:131072,2,32 \
  --vg-cache d1-8k-direct:16384,2,32:8192,1,32:131072,2,32
```

Differences from the cache lab's script, and why:

- **Release build, not Debug.** The graded build is `-O3`; a `-O0` profile describes code that
  is never run and has many times more instructions. The builds add `-g` (debug info only: g++
  generates the same code), so `cg_annotate --auto=yes` still annotates every source line.
- **`--branch-sim=yes`** adds the branch columns (`Bc`, `Bcm`, `Bi`, `Bim`) to the reports.
- **Without `--vg-cache`, misses are for the node's own caches,** not the lab's 16 KB 2-way
  model, so they don't match the lab's numbers. Use `--vg-cache lab:…` for a like-for-like model.

Outputs:

- `avignon-results/<batch>/valgrind.csv`: one row per tag/commit, implementation and cache
  configuration, with instructions (`I_refs`), data accesses (`D_refs`), L1 data misses and rate,
  last-level misses and rate, branches, mispredictions and rate;
- `avignon-results/<batch>/valgrind/<executable>/cachegrind-<cache>.txt` and `callgrind.txt`: the
  annotated reports. `<executable>` is the `binary_id` column of the CSVs. The sources they
  annotate are kept in `test-optimisations/stages/` and `avignon-results/<batch>/src/`, so don't
  delete those before reading the reports.
