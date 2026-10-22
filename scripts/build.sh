#!/bin/sh
#SBATCH --job-name=build
#SBATCH --output=slurm-out/build-%j.out
#SBATCH --error=slurm-out/build-%j.err
# Configure and build the project.

# Usage: sbatch scripts/build.sh [release|debug]   (submit from the repo root)
#        scripts/build.sh [release|debug]          (run directly)
# The slurm-out/ directory must exist before sbatch is called.
set -eu

CONFIG="${1:-release}"
ROOT="${SLURM_SUBMIT_DIR:-$(cd "$(dirname "$0")/.." && pwd)}"
cd "$ROOT"

cmake --preset default
cmake --build --preset "$CONFIG"
echo "Finished building ($CONFIG)"
