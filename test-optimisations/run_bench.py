#!/usr/bin/env python3
"""Times each stage built by make_stages.sh on the 1K and 10K reference cases, and checks every
output against the expected files with tools/compare.py.

Usage: test-optimisations/run_bench.py [--reps N] [--out FILE] [stage ...]   (default: all stages, 5 reps)
Rows are appended to the output CSV (default test-optimisations/results.csv).

Times are wall-clock for the whole program run (input, simulation, output). The runs are done
round-robin (run 1 of every stage, then run 2 of every stage, ...), so a change in the machine's
speed during the benchmark (thermal state, frequency, background work) affects every stage alike
instead of favouring whichever was measured at a good moment. The median is reported.
On a laptop, run it under `caffeinate -i` (macOS) so the machine doesn't sleep.
"""
import argparse
import csv
import platform
import shutil
import statistics
import subprocess
import sys
import tempfile
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parent

# stage -> implementations it changes (only those are timed)
STAGES = {
    "a_restart": ["aos"], "s0_base": ["aos", "soa"], "s1_locals": ["soa"], "s2_onediv": ["soa"],
    "s3_iterators": ["soa"], "a1_forces": ["aos"], "g_grid_only": ["aos", "soa"],
    "x_grid": ["aos", "soa"], "x_flags": ["aos", "soa"], "m_native": ["aos", "soa"],
    "m_fastmath": ["aos", "soa"], "e1_alive": ["aos"], "e2_hoist": ["aos"], "e3_simulate": ["aos"],
}
CASES = ["1k", "10k"]


def round_robin(exes, reps, timeout=None, progress=print):
    """Times every executable on every case, one run of each per round, for `reps` rounds.

    Returns {(exe, case): (status, times)} with status ok | wrong-output | crash | timeout | skipped.
    An executable that fails a run is not run again; one that fails on 1K is not run on 10K."""
    root = Path(tempfile.mkdtemp(dir=str(HERE)))
    try:
        jobs = {}
        for n, exe in enumerate(exes):
            for case in CASES:
                ref = REPO / "ftest/reference" / case
                work = root / "{}-{}".format(n, case)
                (work / "input").mkdir(parents=True)
                shutil.copy(str(ref / "input.txt"), str(work / "input/input.txt"))
                jobs[(exe, case)] = {"work": work, "args": (ref / "args").read_text().split(),
                                     "status": "ok", "times": []}
        for rnd in range(1, reps + 1):
            for exe in exes:
                for case in CASES:
                    job = jobs[(exe, case)]
                    if case != "1k" and jobs[(exe, "1k")]["status"] != "ok":
                        job["status"] = "skipped"
                    if job["status"] != "ok":
                        continue
                    start = time.perf_counter()
                    try:
                        proc = subprocess.run([str(exe)] + job["args"], cwd=str(job["work"]), timeout=timeout,
                                              stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                        job["status"] = "ok" if proc.returncode == 0 else "crash"
                    except subprocess.TimeoutExpired:
                        job["status"] = "timeout"
                    if job["status"] == "ok":
                        job["times"].append(time.perf_counter() - start)
            progress("round {}/{} done".format(rnd, reps))
        results = {}
        for (exe, case), job in jobs.items():
            if job["status"] == "ok":
                expected = REPO / "ftest/reference" / case / "expected"
                compare = [sys.executable, str(REPO / "tools/compare.py"), str(job["work"]), str(expected)]
                if subprocess.run(compare, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode != 0:
                    job["status"] = "wrong-output"
            results[(exe, case)] = (job["status"], job["times"])
        return results
    finally:
        shutil.rmtree(str(root), ignore_errors=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--reps", type=int, default=5)
    parser.add_argument("--out", type=Path, default=HERE / "results.csv")
    parser.add_argument("stages", nargs="*", default=list(STAGES))
    opts = parser.parse_args()

    runs = [(stage, impl, HERE / "stages" / stage / "out/build/default/{0}/Release/grav-{0}".format(impl))
            for stage in opts.stages for impl in STAGES[stage]]
    results = round_robin([exe for _, _, exe in runs], opts.reps)

    new_file = not opts.out.exists()
    with opts.out.open("a", newline="") as fh:
        writer = csv.writer(fh)
        if new_file:
            writer.writerow(["machine", "stage", "impl", "case", "reps", "median_s", "min_s", "max_s", "correct"])
        for stage, impl, exe in runs:
            for case in CASES:
                status, times = results[(exe, case)]
                row = [platform.node(), stage, impl, case, len(times)]
                row += ["{:.3f}".format(v) for v in (statistics.median(times), min(times), max(times))] if times \
                    else ["", "", ""]
                row.append("yes" if status == "ok" else status)
                writer.writerow(row)
                print(*row, flush=True)


if __name__ == "__main__":
    main()
