#!/bin/sh
#SBATCH --job-name=perf
#SBATCH --output=slurm-out/perf-%j.out
#SBATCH --error=slurm-out/perf-%j.err
#SBATCH --exclusive
# Performance and energy measurements. Expects the release build to exist
