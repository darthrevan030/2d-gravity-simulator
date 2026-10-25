#!/usr/bin/env python3
"""Compare two config files (init_config.txt / final_config.txt).

Header width/height/dt and every particle value are compared numerically:
    |a - b| <= max(abs_tol, rel_tol * |b|)
The object count in the header, the number of lines and every object id must
match exactly. Line format (from the instructor's reference files):
    width height time_step count
    id: m r x y vx vy

Default abs_tol = 0.001 (one unit in the last printed decimal), because two
correct implementations can round to neighbouring 3-decimal values.
Use --exact for a byte-for-byte check.

Both arguments can also be directories (e.g. an output folder copied back from
the cluster and a reference folder): init_config.txt and final_config.txt are
compared in each and a PASS/FAIL line is printed per file.
Needs only the Python standard library.

Exit code: 0 = match, 1 = mismatch, 2 = usage / unreadable file.
"""

import argparse
import sys
from pathlib import Path


CONFIG_FILES = ("init_config.txt", "final_config.txt")


def load(path):
    """Return (header tokens, list of particle rows) as floats."""
    try:
        rows = [line.split() for line in Path(path).read_text().splitlines() if line.strip()]
    except OSError as err:
        print(f"compare: {err}", file=sys.stderr)
        sys.exit(2)
    if not rows or len(rows[0]) != 4:
        print(f"compare: {path}: bad or missing header", file=sys.stderr)
        sys.exit(1)
    return rows[0], rows[1:]


def close(a, b, abs_tol, rel_tol):
    return abs(a - b) <= max(abs_tol, rel_tol * abs(b))


def diff(actual, expected, abs_tol, rel_tol):
    """Return the list of mismatch messages (empty = files match)."""
    head_a, rows_a = load(actual)
    head_e, rows_e = load(expected)
    errors = []

    if head_a[3] != head_e[3]:
        errors.append(f"header object count: got {head_a[3]}, expected {head_e[3]}")
    for k, name in enumerate(("width", "height", "time_step")):
        if not close(float(head_a[k]), float(head_e[k]), abs_tol, rel_tol):
            errors.append(f"header {name}: got {head_a[k]}, expected {head_e[k]}")
    if len(rows_a) != len(rows_e):
        errors.append(f"line count: got {len(rows_a)}, expected {len(rows_e)}")

    fields = ("m", "r", "x", "y", "vx", "vy")
    for idx, (ra, re) in enumerate(zip(rows_a, rows_e)):
        # Line format: "id: m r x y vx vy" (id must match exactly)
        if len(ra) != 7 or not ra[0].endswith(":"):
            errors.append(f"line {idx + 2}: expected 'id: m r x y vx vy', got {' '.join(ra)!r}")
            continue
        if ra[0] != re[0]:
            errors.append(f"line {idx + 2}: id {ra[0][:-1]}, expected {re[0][:-1]}")
            continue
        for f, va, ve in zip(fields, ra[1:], re[1:]):
            if not close(float(va), float(ve), abs_tol, rel_tol):
                errors.append(f"id {ra[0][:-1]} {f}: got {va}, expected {ve}")
    return errors


def report(actual, expected, errors):
    print(f"compare: {actual} vs {expected}: {len(errors)} mismatch(es)", file=sys.stderr)
    for e in errors[:10]:
        print(f"  {e}", file=sys.stderr)
    if len(errors) > 10:
        print(f"  ... {len(errors) - 10} more", file=sys.stderr)


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("actual", help="file or directory")
    ap.add_argument("expected", help="file or directory")
    ap.add_argument("--abs", type=float, default=1e-3, dest="abs_tol")
    ap.add_argument("--rel", type=float, default=1e-9, dest="rel_tol")
    ap.add_argument("--exact", action="store_true", help="byte-for-byte comparison")
    ns = ap.parse_args()

    actual, expected = Path(ns.actual), Path(ns.expected)
    if actual.is_dir() != expected.is_dir():
        print("compare: both arguments must be files or both directories", file=sys.stderr)
        sys.exit(2)
    pairs = ([(actual / n, expected / n) for n in CONFIG_FILES]
             if actual.is_dir() else [(actual, expected)])
    verbose = actual.is_dir()

    failed = False
    for a, e in pairs:
        if ns.exact:
            try:
                errors = [] if a.read_bytes() == e.read_bytes() else [f"{a} and {e} differ (exact)"]
            except OSError as err:
                print(f"compare: {err}", file=sys.stderr)
                sys.exit(2)
            if errors and not verbose:
                print(f"compare: {errors[0]}", file=sys.stderr)
        else:
            errors = diff(a, e, ns.abs_tol, ns.rel_tol)
            if errors and not verbose:
                report(a, e, errors)
        if verbose:
            print(f"{'FAIL' if errors else 'PASS'} {a.name}")
            if errors and not ns.exact:
                report(a, e, errors)
        failed = failed or bool(errors)
    sys.exit(1 if failed else 0)


if __name__ == "__main__":
    main()
