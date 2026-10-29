#!/usr/bin/env python3
"""Frontend-side helpers for avignon.sh (works on Python 3.6+).

  stages-manifest --bin-dir DIR --out FILE   list the built stages' executables (one copy of each distinct executable in DIR)
  unique-binaries MANIFEST...                print each distinct executable once, as "<binary> <impl>"
  collect --batch DIR                        join the manifests with the timings from the jobs into
                                             DIR/results.csv (one row per stage/commit, impl and case)
                                             and with the valgrind counts into DIR/valgrind.csv
"""
import argparse
import csv
import math
import re
import subprocess
import sys
from pathlib import Path

from run_bench import STAGES
from timeline import MANIFEST_FIELDS, stripped_copy

HERE = Path(__file__).resolve().parent
REPO = HERE.parent
CASES = ["1k", "10k"]
RESULT_FIELDS = ["kind", "label", "order", "commit", "date", "author", "subject", "impl", "case", "status",
                 "reps", "time_s", "time_sem_pct", "energy_j", "energy_sem_pct", "power_w", "final_objects",
                 "binary_id", "same_binary_as"]
VALGRIND_FIELDS = ["kind", "label", "order", "commit", "date", "author", "subject", "impl", "status",
                   "iterations", "cache", "I1", "D1", "LL", "I_refs", "D_refs", "D1_misses", "D1_miss_rate_pct", "LLd_misses",
                   "LL_miss_rate_pct", "branches", "mispredicts", "mispred_rate_pct", "binary_id",
                   "same_binary_as"]
# cachegrind summary line -> column ("==123== D1  misses:   1,234  (1,000 rd + 234 wr)": first number)
CACHEGRIND_KEYS = {"I refs": "I_refs", "D refs": "D_refs", "D1 misses": "D1_misses",
                   "D1 miss rate": "D1_miss_rate_pct", "LLd misses": "LLd_misses",
                   "LL miss rate": "LL_miss_rate_pct", "Branches": "branches", "Mispredicts": "mispredicts",
                   "Mispred rate": "mispred_rate_pct"}
SUMMARY_LINE = re.compile(r"^==\d+==\s+([A-Za-z0-9 ]+?)\s*:\s+([\d,.]+)%?")


def stages_manifest(opts):
    opts.bin_dir.mkdir(parents=True, exist_ok=True)
    with opts.out.open("w", newline="") as fh:
        writer = csv.DictWriter(fh, fieldnames=MANIFEST_FIELDS)
        writer.writeheader()
        for order, (stage, impls) in enumerate(STAGES.items(), start=1):
            for impl in impls:
                exe = HERE / "stages" / stage / "out/build/default/{0}/Release/grav-{0}".format(impl)
                row = {"kind": "stage", "label": stage, "order": order, "impl": impl,
                       "status": "no-build", "binary": ""}
                if exe.is_file():
                    _, path = stripped_copy(exe, opts.bin_dir)
                    row.update(status="built", binary=str(path))
                writer.writerow(row)


def read_manifests(paths):
    rows = []
    for path in paths:
        if path.exists():
            with path.open() as fh:
                rows.extend(csv.DictReader(fh))
    return rows


def unique_binaries(opts):
    seen = set()
    for row in read_manifests(opts.manifests):
        if row["binary"] and row["binary"] not in seen:
            seen.add(row["binary"])
            print(row["binary"], row["impl"])


def mean_sem_pct(values):
    mean = sum(values) / len(values)
    if len(values) < 2 or mean == 0:
        return mean, 0.0
    var = sum((v - mean) ** 2 for v in values) / (len(values) - 1)
    return mean, math.sqrt(var / len(values)) / mean * 100


def cachegrind_counts(log):
    counts = {}
    for line in log.read_text().splitlines():
        match = SUMMARY_LINE.match(line)
        if match:
            key = " ".join(match.group(1).split())
            if key in CACHEGRIND_KEYS:
                counts[CACHEGRIND_KEYS[key]] = match.group(2).replace(",", "")
    return counts


def collect_valgrind(batch, manifest_rows):
    iterations = (batch / "vg-iterations.txt").read_text().strip()
    # "<name> <I1> <D1> <LL>" per line, "-" = valgrind's model of the node's own caches
    caches = [line.split() for line in (batch / "vg-caches.txt").read_text().splitlines() if line.strip()]
    first_label = {}
    with (batch / "valgrind.csv").open("w", newline="") as fh:
        writer = csv.DictWriter(fh, fieldnames=VALGRIND_FIELDS)
        writer.writeheader()
        for row in manifest_rows:
            base = {k: row.get(k, "") for k in ("kind", "label", "order", "commit", "date", "author",
                                                 "subject", "impl")}
            binary_id = Path(row["binary"]).name if row["binary"] else ""
            if not binary_id:
                writer.writerow(dict(base, status="no-build"))
                continue
            key = (row["kind"], binary_id)
            base.update(binary_id=binary_id, iterations=iterations, same_binary_as=first_label.get(key, ""))
            first_label.setdefault(key, row["label"])
            vg = batch / "valgrind" / binary_id
            status = (vg / "status").read_text().strip() if (vg / "status").exists() else "not-run"
            for name, i1, d1, ll in caches:
                out = dict(base, cache=name, I1=i1 if i1 != "-" else "", D1=d1 if d1 != "-" else "",
                           LL=ll if ll != "-" else "")
                log = vg / "cachegrind-{}.log".format(name)
                if log.exists() and status in ("ok", "timeout (callgrind)"):
                    out.update(cachegrind_counts(log), status="ok")
                else:
                    out["status"] = status
                writer.writerow(out)
    print("valgrind: {}".format(batch / "valgrind.csv"))


def collect(opts):
    batch = opts.batch
    manifest_rows = read_manifests([batch / "stages-manifest.csv", batch / "timeline-manifest.csv"])
    if any((batch / "valgrind").iterdir()):
        collect_valgrind(batch, manifest_rows)
    if not any((batch / "raw").iterdir()):
        return  # --valgrind-only
    # timings written by the jobs: binary_id,case,rep,status,time_s,energy_j
    reps = {}
    for raw in sorted((batch / "raw").glob("*.csv")):
        with raw.open() as fh:
            for binary_id, case, _rep, status, time_s, energy_j in csv.reader(fh):
                reps.setdefault((binary_id, case), []).append((status, time_s, energy_j))

    first_label = {}
    with (batch / "results.csv").open("w", newline="") as fh:
        writer = csv.DictWriter(fh, fieldnames=RESULT_FIELDS)
        writer.writeheader()
        for row in manifest_rows:
            binary_id = Path(row["binary"]).name if row["binary"] else ""
            for case in CASES:
                out = {k: row.get(k, "") for k in ("kind", "label", "order", "commit", "date", "author",
                                                    "subject", "impl")}
                out.update(case=case, binary_id=binary_id)
                if not binary_id:
                    writer.writerow(dict(out, status="no-build"))
                    continue
                key = (row["kind"], binary_id, case)
                if key in first_label:
                    out["same_binary_as"] = first_label[key]
                else:
                    first_label[key] = row["label"]
                runs = reps.get((binary_id, case), [])
                good = [(float(t), float(e)) for status, t, e in runs if status == "ok"]
                if not good:
                    failed = [status for status, _, _ in runs if status != "ok"]
                    writer.writerow(dict(out, status=failed[0] if failed else "not-run"))
                    continue
                time_s, time_sem = mean_sem_pct([t for t, _ in good])
                energy_j, energy_sem = mean_sem_pct([e for _, e in good])
                work = batch / "work" / "{}-{}".format(binary_id, case)
                expected = REPO / "ftest/reference" / case / "expected"
                compare = [sys.executable, str(REPO / "tools/compare.py"), str(work), str(expected)]
                correct = subprocess.run(compare, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL).returncode == 0
                final = work / "final_config.txt"
                objects = final.read_text().split("\n", 1)[0].split()[-1] if final.exists() else ""
                writer.writerow(dict(out, status="ok" if correct else "wrong-output", reps=len(good),
                                     time_s="{:.4f}".format(time_s), time_sem_pct="{:.2f}".format(time_sem),
                                     energy_j="{:.3f}".format(energy_j), energy_sem_pct="{:.2f}".format(energy_sem),
                                     power_w="{:.2f}".format(energy_j / time_s), final_objects=objects))
    print("results: {}".format(batch / "results.csv"))


def main():
    parser = argparse.ArgumentParser()
    sub = parser.add_subparsers(dest="cmd")
    p = sub.add_parser("stages-manifest")
    p.add_argument("--bin-dir", type=Path, required=True)
    p.add_argument("--out", type=Path, required=True)
    p = sub.add_parser("unique-binaries")
    p.add_argument("manifests", type=Path, nargs="+")
    p = sub.add_parser("collect")
    p.add_argument("--batch", type=Path, required=True)
    opts = parser.parse_args()
    {"stages-manifest": stages_manifest, "unique-binaries": unique_binaries, "collect": collect}[opts.cmd](opts)


if __name__ == "__main__":
    main()
