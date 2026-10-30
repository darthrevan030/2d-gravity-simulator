# grav-sim-template
Template for gravity simulation project

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
ctest --preset release --output-on-failure
```

## Project layout

| Folder | Contents |
|---|---|
| `common/` | code shared by both versions: argument parsing, input parser, config writer, physics and collision rules |
| `aos/`, `soa/` | the two implementations, each with its `main.cpp` |
| `utcommon/`, `utaos/`, `utsoa/` | unit tests (GoogleTest) |
| `input/` | example input files |
