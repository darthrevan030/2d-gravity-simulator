# 2D Gravity Simulator

Sequential C++23 simulation of gravitationally interacting objects in a closed
2D enclosure, for the Computer Architecture course at UC3M. There are two
implementations of the same simulation:

- `grav-aos`: particles stored as an array of structures
- `grav-soa`: particles stored as a structure of arrays

The program is sequential. No threads or parallelism are used.

## Requirements

- g++-15
- CMake 4.0 or newer
- Ninja
- clang-tidy (optional, for static analysis)

Dependencies (GSL and GoogleTest) are downloaded automatically at configure time.

## Build

```bash
cmake --preset default
cmake --build --preset release
```

Other build presets: `debug`, `clang-tidy-debug` and `clang-tidy-release`
(the last two need `cmake --preset clang-tidy` first and also run clang-tidy).

The release build uses `-O3 -DNDEBUG -fno-math-errno -fassociative-math
-fno-signed-zeros -fno-trapping-math` (see `CMakePresets.json`).

## Run

```bash
./out/build/default/aos/Release/grav-aos <num_iterations> <width> <height> <time_step>
```

Example: `./grav-aos 100 1000 1000 0.25`

The program reads `input/input.txt` from the current directory. Each line has
six values: mass, radius, x, y, vx, vy. It writes `init_config.txt` (after the
initial collision pass) and `final_config.txt` (after the last iteration) to
the current directory.

## Test

```bash
scripts/utest.sh [release|debug]        # build + all unit tests (same as ctest --preset release)
scripts/ftest.sh [--no-build] [--full]  # functional tests on the real executables
scripts/check_coverage.sh [common|aos|soa]  # coverage report for one unit test target
```

`ftest.sh` runs error cases, hand-made cases (`ftest/cases/`) and the instructor's
reference cases (`ftest/reference/`), comparing outputs with `tools/compare.py`
(values within 0.001). `--full` also runs the 10K reference case (about 1 minute).

## Performance

Time and energy are measured on the avignon cluster with slurm:

```bash
scripts/run_build_perf.sh --reps 3 --tag mytag   # build, then submit the perf jobs
scripts/run_perf.sh --ref-only                   # perf jobs only (release build must exist)
python3 tools/plot.py [perf_<jobid>.csv ...]     # plots (needs matplotlib)
```

Results are in `perf/results/`.

## Project layout

| Folder | Contents |
|---|---|
| `common/` | code shared by both versions: argument parsing, input parser, config writer, physics and collision rules |
| `aos/`, `soa/` | the two implementations, each with its `main.cpp` |
| `utcommon/`, `utaos/`, `utsoa/` | unit tests (GoogleTest) |
| `input/` | example input files |
| `ftest/` | functional test cases and reference outputs |
| `scripts/` | build, test, coverage and cluster performance scripts |
| `tools/` | output comparison (`compare.py`) and plotting (`plot.py`) |
| `perf/` | performance results |
