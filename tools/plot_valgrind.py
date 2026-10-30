#!/usr/bin/env python3
"""Plot cachegrind/branch-sim counters from scripts/run_perf.sh valgrind_*.csv files.

Needs matplotlib (pip install matplotlib). Works from any directory.

Usage:
    tools/plot_valgrind.py [valgrind_<jobid>.csv ...] [-o OUTDIR] [--log]
With no CSV it takes every perf/results/*/valgrind_*.csv, one "level" per folder
(baseline first, then by name). With no -o the plots go to perf/results/plots-valgrind.

One PNG per metric (instructions, data refs, D1 misses and miss rate, branches,
mispredicts and rate, LL misses): bars of AOS vs SOA, one group per level, each
bar labelled with its value. --log uses a log y axis for the count metrics.
"""

import argparse
import csv
from pathlib import Path

try:
    import matplotlib
except ImportError:
    raise SystemExit("plot_valgrind.py: matplotlib is required (pip install matplotlib)")

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent

METRICS = {  # column: (axis label, is a rate)
    "I_refs": ("Instructions executed", False),
    "D_refs": ("Data references", False),
    "D1_misses": ("L1 data cache misses", False),
    "D1_miss_rate_pct": ("L1 data miss rate (%)", True),
    "LLd_misses": ("Last-level data misses", False),
    "branches": ("Branches", False),
    "mispredicts": ("Branch mispredictions", False),
    "mispred_rate_pct": ("Branch misprediction rate (%)", True),
}


def fmt(y):
    return f"{y:.3g}"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("csv", nargs="*", help="default: every perf/results/*/valgrind_*.csv")
    ap.add_argument("-o", "--outdir", default=str(ROOT / "perf" / "results" / "plots-valgrind"))
    ap.add_argument("--log", action="store_true", help="log y axis for count metrics")
    ns = ap.parse_args()

    files = [Path(f) for f in ns.csv]
    if not files:
        files = sorted((ROOT / "perf" / "results").glob("*/valgrind_*.csv"),
                       key=lambda p: ("baseline" not in p.parent.name, p.parent.name))
        if not files:
            raise SystemExit("plot_valgrind.py: no CSV given and none in perf/results/*/")

    levels = []  # (label, {impl: row})
    for f in files:
        try:
            with open(f, newline="", encoding="utf-8") as fh:
                rows = list(csv.DictReader(fh))
        except OSError as err:
            raise SystemExit(f"plot_valgrind.py: {err}")
        levels.append((f.resolve().parent.name, {r["impl"]: r for r in rows}))
    impls = sorted({impl for _, d in levels for impl in d})

    out = Path(ns.outdir)
    out.mkdir(parents=True, exist_ok=True)
    width = 0.8 / len(impls)
    for col, (ylabel, is_rate) in METRICS.items():
        fig, ax = plt.subplots(figsize=(max(6, 2.2 * len(levels) + 2), 4.5))
        for k, impl in enumerate(impls):
            ys = [float(d[impl][col]) if impl in d and d[impl].get(col) not in (None, "") else 0
                  for _, d in levels]
            bars = ax.bar([i + k * width for i in range(len(levels))], ys, width, label=impl.upper())
            ax.bar_label(bars, labels=[fmt(y) for y in ys], fontsize=7, padding=2)
        ax.set_xticks([i + width * (len(impls) - 1) / 2 for i in range(len(levels))])
        ax.set_xticklabels([name for name, _ in levels], rotation=15, ha="right")
        ax.set_ylabel(ylabel)
        ax.set_title(ylabel)
        ax.margins(y=0.15)
        if ns.log and not is_rate:
            ax.set_yscale("log")
        ax.legend()
        fig.tight_layout()
        fig.savefig(out / f"valgrind_{col}.png", dpi=150)
        plt.close(fig)

    print(f"plots written to {out}")


if __name__ == "__main__":
    main()
