#!/usr/bin/env bash
set -euo pipefail

WORK="${TV_WORK_DIR:-$HOME/TerraVision-OG}"
PREFIX="${TV_TCL_PREFIX:-$HOME/opt/terravision-tcl84}"
SOURCE="$WORK/source"
BUILD="$WORK/build"
EXE="$BUILD/TclTV/TclTV"
USER_TV="$HOME/.tv"
CONFIG_SOURCE="$SOURCE/installer/win/tcltk/config"

[[ -x "$EXE" ]] || { echo "TclTV non compilato: esegui 03-build-original-terravision.sh" >&2; exit 1; }
[[ -f "$SOURCE/TclTV/TerraVision.tcl" ]] || { echo "Script Tcl mancanti: $SOURCE/TclTV" >&2; exit 1; }
mkdir -p "$USER_TV/paths"
for f in config.xml database.xml viewpoints.xml; do
  if [[ ! -e "$USER_TV/$f" ]]; then
    cp "$CONFIG_SOURCE/$f" "$USER_TV/$f"
  fi
done
for f in "$CONFIG_SOURCE"/paths/*.path; do
  [[ -e "$USER_TV/paths/$(basename "$f")" ]] || cp "$f" "$USER_TV/paths/"
done

export TVHOME="$SOURCE"
export MAGICHOME="$SOURCE"
export TCL_LIBRARY="$PREFIX/lib/tcl8.4"
export TK_LIBRARY="$PREFIX/lib/tk8.4"
export LD_LIBRARY_PATH="$PREFIX/lib:$BUILD/nsprnew/dist/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
"$(dirname "$0")/05-check-original-build.sh"

if command -v glxinfo >/dev/null 2>&1; then
  echo "OpenGL/GLX:"; glxinfo -B 2>/dev/null | sed -n '1,16p' || true
fi

cd "$SOURCE/TclTV"
exec "$EXE" -tcldir "$SOURCE/TclTV" "$@"
