#!/usr/bin/env python3
"""Builds every commit that touched the code, as it was at that commit (its own sources, presets and
flags), and times grav-aos and grav-soa on the 1K and 10K reference cases. This gives the
performance history of the project, from the first version that runs up to now.

Usage: test-optimisations/timeline.py [--reps N] [--timeout S] [--out FILE] [--branch REF ...]
                                      [--working-tree] [--build-only MANIFEST --bin-dir DIR]
  --reps N        runs per case (default 3; the median is reported)
  --timeout S     give up on one run after S seconds (default 120)
  --branch REF    also include the commits of REF that are not in HEAD (e.g. origin/eda_optimization)
  --working-tree  also measure the uncommitted state of the working tree (labelled "working-tree")
  --build-only M  only build: copy each distinct executable to --bin-dir and write manifest M
                  (used by avignon.sh, which times the executables on the compute nodes)

Output: test-optimisations/timeline.csv, one row per commit, implementation and case, with
status = ok | wrong-output | crash | timeout | skipped | no-build. Every commit is built first;
then each distinct executable is timed, round-robin (one run of each per round, see run_bench.py),
so a drift in the machine's speed affects all commits alike. A commit whose executable is identical
to an earlier one (e.g. an SOA-only commit, for grav-aos) repeats the earlier times and says
"same-binary-as <commit>". Outputs are checked with tools/compare.py against
ftest/reference/<case>/expected (values within 0.001). On a laptop, run it under `caffeinate -i`.
"""
import argparse
import csv
import hashlib
import os
import shutil
import statistics
import subprocess
import sys
import tempfile
import time
from pathlib import Path

from run_bench import CASES, round_robin

HERE = Path(__file__).resolve().parent
REPO = HERE.parent
DEPS = REPO / "out/build/default/_deps"
CODE_PATHS = ["aos", "soa", "common", "CMakeLists.txt", "CMakePresets.json"]
IMPLS = ["aos", "soa"]
FIELDS = ["order", "commit", "date", "author", "subject", "impl", "case", "status",
          "median_s", "min_s", "max_s", "reps", "note"]
MANIFEST_FIELDS = ["kind", "label", "order", "commit", "date", "author", "subject", "impl", "status", "binary"]
WORKING_TREE = "uncommitted changes in the working tree"


def run(cmd, **kwargs):
    """subprocess.run with output captured as text (works on Python 3.6)."""
    return subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, universal_newlines=True, **kwargs)


def git(*args):
    proc = run(["git", "-C", str(REPO)] + list(args))
    if proc.returncode != 0:
        sys.exit("git {} failed: {}".format(" ".join(args), proc.stderr.strip()))
    return proc.stdout


def commits_to_measure(branches):
    """Every commit touching the code reachable from HEAD (plus extra branches), oldest first."""
    return git("rev-list", "--reverse", "--topo-order", "HEAD", *branches, "--", *CODE_PATHS).split()


def describe(commit):
    date, author, subject = git("log", "-1", "--format=%ad%x1f%an%x1f%s", "--date=short", commit).strip().split("\x1f")
    return date, author, subject


def build(commit, tree):
    """Exports `commit` into `tree` and builds it; returns {impl: executable} for what was built."""
    shutil.rmtree(str(tree), ignore_errors=True)
    tree.mkdir(parents=True)
    archive = subprocess.run(["git", "-C", str(REPO), "archive", commit], stdout=subprocess.PIPE, check=True).stdout
    subprocess.run(["tar", "-x", "-C", str(tree)], input=archive, check=True)
    # -g only adds debug info (g++ generates the same code), so valgrind can annotate source lines;
    # nothing in the project sets CMAKE_CXX_FLAGS, so this doesn't override any commit's own flags
    configure = ["cmake", "--preset", "default", "-DCMAKE_CXX_FLAGS=-g"]
    if (DEPS / "gsl-src").is_dir():  # reuse the downloaded libraries instead of fetching them again
        configure += ["-DFETCHCONTENT_FULLY_DISCONNECTED=ON",
                      "-DFETCHCONTENT_SOURCE_DIR_GSL={}".format(DEPS / "gsl-src"),
                      "-DFETCHCONTENT_SOURCE_DIR_GOOGLETEST={}".format(DEPS / "googletest-src")]
    with (tree / "build.log").open("w") as log:
        if subprocess.run(configure, cwd=str(tree), stdout=log, stderr=subprocess.STDOUT).returncode != 0:
            return {}
        built = {}
        for impl in IMPLS:  # one target at a time, so one broken target does not hide the other
            cmd = ["cmake", "--build", "--preset", "release", "--target", "grav-" + impl]
            subprocess.run(cmd, cwd=str(tree), stdout=log, stderr=subprocess.STDOUT)
            exe = tree / "out/build/default/{0}/Release/grav-{0}".format(impl)
            if exe.is_file():
                built[impl] = exe
        return built


def stripped_copy(exe, dest_dir):
    """Copies `exe` to dest_dir, named by the hash of its stripped code; returns (hash, path).

    The hash is taken without symbols, because those include the build paths: the same code built
    in two different folders then gets the same name, and identical executables are measured only
    once. The copy itself keeps its symbols, so callgrind can name the functions."""
    tmp = dest_dir / (exe.name + ".tmp")
    shutil.copy(str(exe), str(tmp))
    run(["strip", str(tmp)])
    digest = hashlib.sha256(tmp.read_bytes()).hexdigest()[:16]
    tmp.unlink()
    final = dest_dir / "{}-{}".format(digest, exe.name)
    if not final.exists():
        shutil.copy(str(exe), str(final))
    return digest, final


def todo_list(opts):
    todo = [(c,) + describe(c) for c in commits_to_measure(opts.branch)]
    if opts.working_tree:
        stash = git("stash", "create").strip()  # a commit object of the working tree; changes nothing
        if stash:
            todo.append((stash, time.strftime("%Y-%m-%d"), "", WORKING_TREE))
    return todo


def short_name(commit, subject):
    return "working-tree" if subject == WORKING_TREE else commit[:7]


def build_only(opts):
    """Builds every commit and writes a manifest; the timing is done elsewhere (avignon.sh)."""
    opts.bin_dir.mkdir(parents=True, exist_ok=True)
    trees = opts.build_only.parent / "src"  # each commit's sources stay here for cg_annotate
    todo = todo_list(opts)
    with opts.build_only.open("w", newline="") as fh:
        writer = csv.DictWriter(fh, fieldnames=MANIFEST_FIELDS)
        writer.writeheader()
        for order, (commit, date, author, subject) in enumerate(todo, start=1):
            short = short_name(commit, subject)
            tree = trees / short
            built = build(commit, tree)
            status = []
            for impl in IMPLS:
                row = {"kind": "commit", "label": short, "order": order, "commit": short, "date": date,
                       "author": author, "subject": subject, "impl": impl, "status": "no-build", "binary": ""}
                if impl in built:
                    _, path = stripped_copy(built[impl], opts.bin_dir)
                    row.update(status="built", binary=str(path))
                writer.writerow(row)
                status.append("{} {}".format(impl, row["status"]))
            fh.flush()
            shutil.rmtree(str(tree / "out"), ignore_errors=True)  # keep the sources, drop the build
            print("[{}/{}] {} {}  ({})".format(order, len(todo), short, subject[:55], ", ".join(status)), flush=True)


def measure(opts):
    """Builds every commit, then times each distinct executable round-robin (see run_bench.py)."""
    bins = Path(tempfile.mkdtemp(dir=str(HERE)))
    tree = HERE / "timeline" / "build"
    try:
        todo = todo_list(opts)
        built_at = []  # (order, short, date, author, subject, {impl: executable})
        for order, (commit, date, author, subject) in enumerate(todo, start=1):
            short = short_name(commit, subject)
            built = build(commit, tree)
            exes = {impl: stripped_copy(exe, bins)[1] for impl, exe in built.items()}
            built_at.append((order, short, date, author, subject, exes))
            print("[{}/{}] built {} {}  ({})".format(order, len(todo), short, subject[:50],
                                                     ", ".join(sorted(exes)) or "nothing"), flush=True)
        distinct = []
        for *_, exes in built_at:
            distinct += [exe for exe in exes.values() if exe not in distinct]
        print("timing {} distinct executables, {} rounds".format(len(distinct), opts.reps), flush=True)
        results = round_robin(distinct, opts.reps, opts.timeout, progress=lambda m: print(m, flush=True))

        first_seen = {}
        with opts.out.open("w", newline="") as fh:
            writer = csv.DictWriter(fh, fieldnames=FIELDS)
            writer.writeheader()
            for order, short, date, author, subject, exes in built_at:
                for impl in IMPLS:
                    base = {"order": order, "commit": short, "date": date, "author": author,
                            "subject": subject, "impl": impl}
                    exe = exes.get(impl)
                    if exe is None:
                        for case in CASES:
                            writer.writerow(dict(base, case=case, status="no-build"))
                        continue
                    note = "same-binary-as " + first_seen[exe] if exe in first_seen else ""
                    first_seen.setdefault(exe, short)
                    for case in CASES:
                        status, times = results[(exe, case)]
                        row = dict(base, case=case, status=status, note=note, reps=len(times))
                        if times:
                            row.update(median_s="{:.3f}".format(statistics.median(times)),
                                       min_s="{:.3f}".format(min(times)), max_s="{:.3f}".format(max(times)))
                        writer.writerow(row)
        print("written: {}".format(opts.out))
    finally:
        shutil.rmtree(str(bins), ignore_errors=True)
        shutil.rmtree(str(tree), ignore_errors=True)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--reps", type=int, default=3)
    parser.add_argument("--timeout", type=float, default=120)
    parser.add_argument("--out", type=Path, default=HERE / "timeline.csv")
    parser.add_argument("--branch", action="append", default=[])
    parser.add_argument("--working-tree", action="store_true")
    parser.add_argument("--build-only", type=Path)
    parser.add_argument("--bin-dir", type=Path)
    opts = parser.parse_args()
    if opts.build_only:
        if not opts.bin_dir:
            parser.error("--build-only needs --bin-dir")
        build_only(opts)
    else:
        measure(opts)


if __name__ == "__main__":
    main()
