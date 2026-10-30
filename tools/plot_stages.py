#!/usr/bin/env python3
"""Compare optimisation stages using scripts/run_perf.sh perf_*.csv files.

Needs matplotlib (pip install matplotlib). Works from any directory.

Usage:
    tools/plot_stages.py [perf_<jobid>.csv ...] [-o OUTDIR] [--log]
Each CSV is one stage, named after its folder with a trailing -perf/-scalability
removed (baseline-preoptimisation, optimisations-1, ...). With no CSV it uses
perf/results/*/, skipping folders with "test" in the name and, per stage, taking
the CSV with the most rows (the scalability run, which also has the reference
cases). With no -o the plots go to perf/results/plots-stages.

Produces, for each metric (time, energy, power):
  stages_reference_<metric>.png   bars per stage, AOS vs SOA, 1K and 10K panels
  stages_scale_n_<metric>.png     lines per stage vs number of objects (AOS | SOA)
  stages_scale_iter_<metric>.png  lines per stage vs iterations (AOS | SOA)
and for time and energy:
  stages_speedup_<metric>.png     reference cases, baseline / stage per implementation
Every point and bar is labelled. --log uses log axes on the scalability lines.
"""

import argparse
import csv
import re
from pathlib import Path

try:
    import matplotlib
except ImportError:
    raise SystemExit("plot_stages.py: matplotlib is required (pip install matplotlib)")

matplotlib.use("Agg")
import matplotlib.pyplot as plt  # noqa: E402

ROOT = Path(__file__).resolve().parent.parent

METRICS = {  # name: (column, axis label)
    "time": ("time_s", "Execution time (s)"),
    "energy": ("energy_j", "Package energy (J)"),
    "power": ("power_w", "Average power (W)"),
}
OFFSETS = [(0, 6), (0, -10), (0, -10), (0, 14)]  # label offsets, one per stage
IMPLS = ("aos", "soa")


def fmt(y):
    return f"{y:.3g}"


def stage_name(f):
    return re.sub(r"-(perf|scalability)$", "", f.resolve().parent.name)


def stage_order(name):
    return ("baseline" not in name, name)


def load(f):
    try:
        with open(f, newline="", encoding="utf-8") as fh:
            return list(csv.DictReader(fh))
    except OSError as err:
        raise SystemExit(f"plot_stages.py: {err}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("csv", nargs="*", help="default: the fullest perf_*.csv of each perf/results/*/")
    ap.add_argument("-o", "--outdir", default=str(ROOT / "perf" / "results" / "plots-stages"))
    ap.add_argument("--log", action="store_true", help="log axes on the scalability lines")
    ns = ap.parse_args()

    stages = {}  # name -> rows
    if ns.csv:
        for f in map(Path, ns.csv):
            stages.setdefault(stage_name(f), []).extend(load(f))
    else:
        for f in (ROOT / "perf" / "results").glob("*/perf_*.csv"):
            if "test" in f.parent.name:
                continue
            rows = load(f)
            if len(rows) > len(stages.get(stage_name(f), [])):
                stages[stage_name(f)] = rows
    if not stages:
        raise SystemExit("plot_stages.py: no CSV given and none in perf/results/*/")
    names = sorted(stages, key=stage_order)
    print("stages:", ", ".join(names))

    def pick(name, impl, scen, **match):
        return {int(r["n"] if scen != "scale_iter" else r["iterations"]): r
                for r in stages[name]
                if r["impl"] == impl and r["scenario"] == scen
                and all(r[k] == str(v) for k, v in match.items())}

    out = Path(ns.outdir)
    out.mkdir(parents=True, exist_ok=True)
    colors = {n: f"C{i}" for i, n in enumerate(names)}
    sizes = sorted({int(r["n"]) for n in names for r in stages[n] if r["scenario"] == "reference"})

    def ref_value(name, impl, n, col):
        r = pick(name, impl, "reference").get(n)
        return float(r[col]) if r else None

    for metric, (col, ylabel) in METRICS.items():
        # Reference cases: one panel per size, stages on x, AOS/SOA bars
        if sizes:
            fig, axes = plt.subplots(1, len(sizes), figsize=(4.5 * len(sizes) + 1, 4.5), squeeze=False)
            for ax, n in zip(axes[0], sizes):
                width = 0.8 / len(IMPLS)
                for k, impl in enumerate(IMPLS):
                    ys = [ref_value(s, impl, n, col) or 0 for s in names]
                    bars = ax.bar([i + k * width for i in range(len(names))], ys, width, label=impl.upper())
                    ax.bar_label(bars, labels=[fmt(y) for y in ys], fontsize=7, padding=2)
                ax.set_xticks([i + width * (len(IMPLS) - 1) / 2 for i in range(len(names))])
                ax.set_xticklabels(names, rotation=15, ha="right")
                ax.set_title(f"{n // 1000}K objects")
                ax.margins(y=0.15)
            axes[0][0].set_ylabel(ylabel)
            axes[0][0].legend()
            fig.suptitle(f"Stages, reference cases: {ylabel.split(' (')[0].lower()}")
            fig.tight_layout()
            fig.savefig(out / f"stages_reference_{metric}.png", dpi=150)
            plt.close(fig)

        # Scalability: one panel per implementation, one line per stage
        for scen, xlabel, fixed in (("scale_n", "Number of objects", {}),
                                    ("scale_iter", "Iterations", {"n": 1000})):
            if not any(r["scenario"] == scen for n in names for r in stages[n]):
                continue
            fig, axes = plt.subplots(1, len(IMPLS), figsize=(5.5 * len(IMPLS), 4.5), sharey=True)
            for ax, impl in zip(axes, IMPLS):
                for k, name in enumerate(names):
                    pts = sorted(pick(name, impl, scen, **fixed).items())
                    if not pts:
                        continue
                    xs = [x for x, _ in pts]
                    ys = [float(r[col]) for _, r in pts]
                    ax.plot(xs, ys, marker="o", color=colors[name], label=name)
                    for x, y in zip(xs, ys):
                        ax.annotate(fmt(y), (x, y), textcoords="offset points",
                                    xytext=OFFSETS[k % len(OFFSETS)], ha="center", fontsize=6,
                                    color=colors[name])
                ax.set_title(impl.upper())
                ax.set_xlabel(xlabel)
                ax.grid(True, alpha=0.3)
                ax.margins(y=0.15)
                if ns.log:
                    ax.set_xscale("log")
                    ax.set_yscale("log")
            axes[0].set_ylabel(ylabel)
            axes[0].legend()
            fig.suptitle(f"Stages, {xlabel.lower()}: {ylabel.split(' (')[0].lower()}")
            fig.tight_layout()
            fig.savefig(out / f"stages_{scen}_{metric}.png", dpi=150)
            plt.close(fig)

    # Speedup over the first stage (lower is better for time and energy)
    base = names[0]
    for metric in ("time", "energy"):
        col, ylabel = METRICS[metric]
        if not sizes or len(names) < 2:
            break
        fig, axes = plt.subplots(1, len(sizes), figsize=(4.5 * len(sizes) + 1, 4.5),
                                 sharey=True, squeeze=False)
        for ax, n in zip(axes[0], sizes):
            width = 0.8 / len(IMPLS)
            for k, impl in enumerate(IMPLS):
                b = ref_value(base, impl, n, col)
                ys = [b / v if b and (v := ref_value(s, impl, n, col)) else 0 for s in names]
                bars = ax.bar([i + k * width for i in range(len(names))], ys, width, label=impl.upper())
                ax.bar_label(bars, labels=[f"{y:.2f}x" for y in ys], fontsize=7, padding=2)
            ax.axhline(1, color="gray", lw=0.8, ls="--")
            ax.set_xticks([i + width * (len(IMPLS) - 1) / 2 for i in range(len(names))])
            ax.set_xticklabels(names, rotation=15, ha="right")
            ax.set_title(f"{n // 1000}K objects")
            ax.margins(y=0.15)
        axes[0][0].set_ylabel(f"{ylabel.split(' (')[0]} speedup vs {base}")
        axes[0][0].legend()
        fig.suptitle(f"Speedup over {base}: {ylabel.split(' (')[0].lower()}")
        fig.tight_layout()
        fig.savefig(out / f"stages_speedup_{metric}.png", dpi=150)
        plt.close(fig)

    print(f"plots written to {out}")


if __name__ == "__main__":
    main()
