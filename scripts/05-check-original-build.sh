#!/usr/bin/env bash
set -euo pipefail

WORK="${TV_WORK_DIR:-$HOME/TerraVision-OG}"
PREFIX="${TV_TCL_PREFIX:-$HOME/opt/terravision-tcl84}"
EXE="$WORK/build/TclTV/TclTV"
export LD_LIBRARY_PATH="$PREFIX/lib:$WORK/build/nsprnew/dist/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

[[ -x "$EXE" ]] || { echo "TclTV non trovato: $EXE" >&2; exit 1; }
file "$EXE" | tee /tmp/terravision-file-check.txt
grep -q 'ELF 32-bit' /tmp/terravision-file-check.txt || {
  echo "ERRORE: TclTV non è un eseguibile ELF a 32 bit." >&2
  exit 1
}

if command -v ldd >/dev/null 2>&1; then
  echo
  echo "Dipendenze dinamiche:"
  DEPS="$(ldd "$EXE" 2>&1 || true)"
  printf '%s\n' "$DEPS"
  if grep -q 'not found' <<<"$DEPS"; then
    echo "ERRORE: una o più librerie runtime non sono installate." >&2
    exit 1
  fi
fi

echo
echo "OK: file ELF i386 presente e nessuna dipendenza dinamica indicata come mancante."
