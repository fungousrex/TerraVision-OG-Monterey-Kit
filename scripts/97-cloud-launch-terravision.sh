#!/usr/bin/env bash
set -euo pipefail

KIT="$(cd "$(dirname "$0")/.." && pwd)"
WORK="${TV_WORK_DIR:-$HOME/TerraVision-OG}"
PREFIX="${TV_TCL_PREFIX:-$HOME/opt/terravision-tcl84}"
EXE="$WORK/build/TclTV/TclTV"

[[ -x "$EXE" ]] || {
  echo "TclTV non risulta compilato. Prima esegui: bash scripts/99-build-original.sh" >&2
  exit 1
}
[[ -f "$PREFIX/lib/tclConfig.sh" && -f "$PREFIX/lib/tkConfig.sh" ]] || {
  echo "Runtime Tcl/Tk 8.4 non trovato in $PREFIX" >&2; exit 1;
}
export DISPLAY="${DISPLAY:-:0}"
export LIBGL_ALWAYS_SOFTWARE=1
export TVHOME="$WORK/source"
export MAGICHOME="$WORK/source"
export TCL_LIBRARY="$PREFIX/lib/tcl8.4"
export TK_LIBRARY="$PREFIX/lib/tk8.4"
export LD_LIBRARY_PATH="$PREFIX/lib:$WORK/build/nsprnew/dist/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

exec bash "$KIT/scripts/04-run-original-terravision.sh"
