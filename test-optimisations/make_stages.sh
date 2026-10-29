#!/usr/bin/env bash
# Builds one source tree per optimisation stage into test-optimisations/stages/, straight from git
# objects, so the working tree is never touched. Each stage is the pre-optimisation code (03e761e)
# with some later changes layered on top; see README.md for what each stage contains.
#
# Usage: test-optimisations/make_stages.sh [stage ...]   (default: all stages)
# Env:   CXX (default g++-15)
# Needs the commits 03e761e, 7fe1cfd, 57af213 and origin/eda_optimization (run `git fetch` first).
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/.." && pwd)"
STAGES="$HERE/stages"
DEPS="$REPO/out/build/default/_deps"  # reused if present, so no download is needed
BASE=03e761e   # main before the optimisation pass
FINAL=57af213  # all code changes + compiler flags
EDA=origin/eda_optimization
export CXX="${CXX:-g++-15}"
mkdir -p "$STAGES"

export_base() {  # $1 = stage dir
  rm -rf "$1"; mkdir -p "$1"
  git -C "$REPO" archive "$BASE" | tar -x -C "$1"
}
take() {  # $1 = stage dir, $2 = commit, $3.. = files to copy from that commit
  local d="$1" c="$2"; shift 2
  for f in "$@"; do mkdir -p "$d/$(dirname "$f")"; git -C "$REPO" show "$c:$f" >"$d/$f"; done
}
edit() {  # $1 = file, $2 = perl substitution; fails if nothing changed
  local before; before="$(cat "$1")"
  perl -0pi -e "$2" "$1"
  [ "$before" != "$(cat "$1")" ] || { echo "ERROR: edit did not apply to $1: $2" >&2; exit 1; }
}
soa_locals() {  # SOA change 1 alone: sum particle i's force in locals (zip | drop view and 2 divisions kept)
  local f="$1/soa/src/integrate.cpp"
  edit "$f" 's/double & fxi\s*= gsl::at\(system\.ax, i\);[^\n]*/double fxi = 0.0;/'
  edit "$f" 's/double & fyi\s*= gsl::at\(system\.ay, i\);/double fyi = 0.0;/'
  edit "$f" 's/(fyj -= fy;\n\s*\}\n)/$1            gsl::at(system.ax, i) += fxi;\n            gsl::at(system.ay, i) += fyi;\n/'
}
soa_one_div() {  # SOA change 2: one division per pair instead of two
  local f="$1/soa/src/integrate.cpp"
  edit "$f" 's/double const g_masses\s*= gravitational_constant \* mi \* mj;/double const scale = gravitational_constant * mi * mj \/ dist_cubed;/'
  edit "$f" 's/g_masses \* dx \/ dist_cubed/scale * dx/; s/g_masses \* dy \/ dist_cubed/scale * dy/'
}
final_code() {  # every code change of the optimisation pass (force loops + collision grid), no flags
  take "$1" "$FINAL" soa/src/integrate.cpp aos/src/integrate.cpp aos/src/collisions.cpp soa/src/collisions.cpp \
    common/include/collision_grid.hpp common/src/collision_grid.cpp common/CMakeLists.txt
}
flags() { take "$1" "$FINAL" CMakePresets.json; }

make_stage() {  # $1 = stage name
  local d="$STAGES/$1"
  case "$1" in
    # SOA chain
    s0_base)        export_base "$d" ;;
    s1_locals)      export_base "$d"; soa_locals "$d" ;;
    s2_onediv)      export_base "$d"; soa_locals "$d"; soa_one_div "$d" ;;
    s3_iterators)   export_base "$d"; take "$d" "$FINAL" soa/src/integrate.cpp ;;
    # AOS chain
    a_restart)      export_base "$d"; take "$d" 7fe1cfd^ aos/src/collisions.cpp ;;
    a1_forces)      export_base "$d"; take "$d" "$FINAL" aos/src/integrate.cpp ;;
    # both
    g_grid_only)    export_base "$d"; take "$d" "$FINAL" aos/src/collisions.cpp soa/src/collisions.cpp \
                      common/include/collision_grid.hpp common/src/collision_grid.cpp common/CMakeLists.txt ;;
    x_grid)         export_base "$d"; final_code "$d" ;;
    x_flags)        export_base "$d"; final_code "$d"; flags "$d" ;;
    m_native)       export_base "$d"; final_code "$d"; flags "$d"
                    edit "$d/CMakePresets.json" 's/-fno-trapping-math"/-fno-trapping-math -march=native"/' ;;
    m_fastmath)     export_base "$d"; final_code "$d"; flags "$d"
                    edit "$d/CMakePresets.json" 's/-fno-math-errno -fassociative-math -fno-signed-zeros -fno-trapping-math/-ffast-math/' ;;
    # eda_optimization branch (ergin04eda), each measured on its own
    # 958d05d, not the branch tip: 765f7e6 drops a static_cast and no longer builds with -Werror
    e1_alive)       export_base "$d"; take "$d" 958d05d aos/src/collisions.cpp ;;  # vs s0_base (tombstones)
    e2_hoist)       export_base "$d"; take "$d" "$EDA" aos/src/integrate.cpp ;;    # vs s0_base
    e3_simulate)    export_base "$d"; final_code "$d"; flags "$d"; take "$d" "$EDA" aos/src/particle_system.cpp ;;  # vs x_flags
    *) echo "ERROR: unknown stage $1" >&2; exit 2 ;;
  esac
}

ALL="s0_base s1_locals s2_onediv s3_iterators a_restart a1_forces g_grid_only x_grid x_flags m_native m_fastmath
     e1_alive e2_hoist e3_simulate"
for name in ${*:-$ALL}; do
  make_stage "$name"
  d="$STAGES/$name"
  fetch=()
  [ -d "$DEPS/gsl-src" ] && fetch=(-DFETCHCONTENT_FULLY_DISCONNECTED=ON -DFETCHCONTENT_SOURCE_DIR_GSL="$DEPS/gsl-src"
                                   -DFETCHCONTENT_SOURCE_DIR_GOOGLETEST="$DEPS/googletest-src")
  if (cd "$d" && cmake --preset default "${fetch[@]}" >"$d/configure.log" 2>&1 \
        && cmake --build --preset release --target grav-aos grav-soa >"$d/build.log" 2>&1); then
    warnings=$(grep -c "warning:" "$d/build.log" || true)
    echo "built $name (warnings: $warnings)"
  else
    echo "FAILED $name (see $d/build.log)"
  fi
done
