#!/usr/bin/env bash
# Build the project and run all unit tests (utcommon, utaos, utsoa).
#
# Usage: ./utest.sh [release|debug]     (default: release)
# Exit code: 0 if everything builds and every test passes, non-zero otherwise.
set -euo pipefail

CONFIG="${1:-release}"
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

echo "== Configure"
cmake --preset default >/dev/null

echo "== Build ($CONFIG)"
cmake --build --preset "$CONFIG"

echo "== Unit tests ($CONFIG)"
ctest --preset "$CONFIG" --output-on-failure
