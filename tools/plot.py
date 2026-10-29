#!/usr/bin/env python3
"""Plot time, energy and power from scripts/run_perf.sh CSV files.

Meant to run locally on a copy of the CSVs from the cluster (needs matplotlib:
pip install matplotlib). Works from any directory.

Usage:
    perf/plot.py [perf_<jobid>.csv ...] [-o OUTDIR]
With no CSV it uses the newest perf/results/perf_*.csv; with no -o the plots go
to perf/results/plots. CSVs from different jobs that share a tag are told apart
by their job id.

Produces, for each metric (time, energy, power):
  reference_<metric>.png   bars: AOS vs SOA on the 1K and 10K reference cases
  scale_n_<metric>.png     lines: metric vs number of objects
  scale_iter_<metric>.png  lines: metric vs iterations
One series per implementation and tag, so "baseline" vs "optimized" runs
can be shown together. Error bars = standard error of the mean from perf stat.
"""

import argparse
import csv
from pathlib import Path

try:
    import matplotlib
except ImportError:
    raise SystemExit("plot.py: matplotlib is required (pip install matplotlib)")

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent

METRICS = {  # name: (column, rsd column, axis label)
    "time": ("time_s", "time_rsd_pct", "Execution time (s)"),
    "energy": ("energy_j", "energy_rsd_pct", "Package energy (J)"),
    "power": ("power_w", None, "Average power (W)"),
}


def value(row, col):
    return float(row[col]) if col and row.get(col) not in (None, "") else 0.0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("csv", nargs="*", help="default: newest perf/results/perf_*.csv")
    ap.add_argument("-o", "--outdir", default=str(ROOT / "perf" / "results" / "plots"))
    ns = ap.parse_args()

    files = [Path(f) for f in ns.csv]
    if not files:
        found = sorted((ROOT / "perf" / "results").glob("perf_*.csv"), key=lambda p: p.stat().st_mtime)
        if not found:
            raise SystemExit("plot.py: no CSV given and none in perf/results/")
        files = [found[-1]]
        print(f"using {files[0]}")

    rows = []
    tag_files = {}  # tag -> set of files using it
    for f in files:
        try:
            with open(f, newline="", encoding="utf-8") as fh:
                part = list(csv.DictReader(fh))
        except OSError as err:
            raise SystemExit(f"plot.py: {err}")
        for r in part:
            r["_job"] = f.stem.removeprefix("perf_")
            tag_files.setdefault(r["tag"], set()).add(f)
        rows += part
    for r in rows:  # same tag in several jobs: keep the series apart
        if len(tag_files[r["tag"]]) > 1:
            r["tag"] = f"{r['tag']} #{r['_job']}"
    out = Path(ns.outdir)
    out.mkdir(parents=True, exist_ok=True)
    series = sorted({(r["impl"], r["tag"]) for r in rows})
    single_tag = len({tag for _, tag in series}) == 1

    def label(impl, tag):
        return impl.upper() if single_tag else f"{impl.upper()} ({tag})"

    for metric, (col, rsd, ylabel) in METRICS.items():
        # Reference cases: grouped bars
        ref = [r for r in rows if r["scenario"] == "reference"]
        if ref:
            sizes = sorted({int(r["n"]) for r in ref})
            fig, ax = plt.subplots(figsize=(6, 4))
            width = 0.8 / len(series)
            for k, (impl, tag) in enumerate(series):
                pts = {int(r["n"]): r for r in ref if (r["impl"], r["tag"]) == (impl, tag)}
                ys = [value(pts[n], col) if n in pts else 0 for n in sizes]
                errs = [value(pts[n], rsd) / 100 * y if n in pts else 0 for n, y in zip(sizes, ys)]
                ax.bar([i + k * width for i in range(len(sizes))], ys, width,
                       yerr=errs if rsd else None, capsize=3, label=label(impl, tag))
            ax.set_xticks([i + width * (len(series) - 1) / 2 for i in range(len(sizes))])
            ax.set_xticklabels([f"{n // 1000}K" for n in sizes])
            ax.set_ylabel(ylabel)
            ax.set_title(f"Reference cases: {ylabel.split(' (')[0].lower()}")
            ax.legend()
            fig.tight_layout()
            fig.savefig(out / f"reference_{metric}.png", dpi=150)
            plt.close(fig)

        # Scalability: lines
        for scen, xcol, xlabel in (("scale_n", "n", "Number of objects"),
                                   ("scale_iter", "iterations", "Iterations")):
            pts_all = [r for r in rows if r["scenario"] == scen]
            if not pts_all:
                continue
            fig, ax = plt.subplots(figsize=(6, 4))
            for impl, tag in series:
                pts = sorted((r for r in pts_all if (r["impl"], r["tag"]) == (impl, tag)),
                             key=lambda r: int(r[xcol]))
                xs = [int(r[xcol]) for r in pts]
                ys = [value(r, col) for r in pts]
                errs = [value(r, rsd) / 100 * y for r, y in zip(pts, ys)] if rsd else None
                ax.errorbar(xs, ys, yerr=errs, marker="o", capsize=3, label=label(impl, tag))
            ax.set_xlabel(xlabel)
            ax.set_ylabel(ylabel)
            ax.set_title(f"{xlabel}: {ylabel.split(' (')[0].lower()}")
            ax.grid(True, alpha=0.3)
            ax.legend()
            fig.tight_layout()
            fig.savefig(out / f"{scen}_{metric}.png", dpi=150)
            plt.close(fig)

    print(f"plots written to {out}")


if __name__ == "__main__":
    main()
