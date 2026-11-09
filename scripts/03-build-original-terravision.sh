#!/usr/bin/env bash
set -euo pipefail

KIT="$(cd "$(dirname "$0")/.." && pwd)"
WORK="${TV_WORK_DIR:-$HOME/TerraVision-OG}"
PREFIX="${TV_TCL_PREFIX:-$HOME/opt/terravision-tcl84}"
SOURCE_IN_KIT="$KIT/Source/TerraVision-6.0-HEAD"
SOURCE="$WORK/source"
BUILD="$WORK/build"
LOG="$WORK/build-terravision.log"
JOBS="${TV_JOBS:-1}"

[[ -f "$SOURCE_IN_KIT/configure" ]] || { echo "Sorgente non trovato: $SOURCE_IN_KIT" >&2; exit 1; }
[[ -f "$PREFIX/lib/tclConfig.sh" && -f "$PREFIX/lib/tkConfig.sh" ]] || {
  echo "Prima esegui scripts/02-build-tcltk84.sh" >&2; exit 1;
}
"$(dirname "$0")/01-check-32bit.sh"

if [[ ! -d "$SOURCE" ]]; then
  mkdir -p "$WORK"
  cp -a "$SOURCE_IN_KIT" "$SOURCE"
fi
# Le revisioni RCS ricostruite hanno permessi storici anche read-only.
# Rendi scrivibile SOLO la working copy, mai lo snapshot incluso nel kit.
chmod -R u+rwX "$SOURCE"
mkdir -p "$BUILD"

# Il makefile storico NSPR Linux.mk forza CC=gcc e CCC=g++.
# Queste due righe della COPIA DI LAVORO vengono adattate all'ABI i386;
# la cartella Source del kit resta intatta e rappresenta lo snapshot originale.
NSPR_LINUX_MK="$SOURCE/nsprnew/config/Linux.mk"
if ! grep -Eq '^CC[[:space:]]*=[[:space:]]*gcc -m32$' "$NSPR_LINUX_MK"; then
  sed -i -E 's/^CC[[:space:]]*=[[:space:]]*gcc[[:space:]]*$/CC\t\t\t= gcc -m32/' "$NSPR_LINUX_MK"
fi
if ! grep -Eq '^CCC[[:space:]]*=[[:space:]]*g\+\+ -m32$' "$NSPR_LINUX_MK"; then
  sed -i -E 's/^CCC[[:space:]]*=[[:space:]]*g\+[+][[:space:]]*$/CCC\t\t\t= g++ -m32/' "$NSPR_LINUX_MK"
fi

export CC='gcc -m32'
export CXX='g++ -m32'
export CFLAGS='-O2 -m32 -fcommon'
export CXXFLAGS='-O2 -m32 -fcommon -std=gnu++98'
export LDFLAGS="-m32 -L$PREFIX/lib -Wl,-rpath,$PREFIX/lib"
export LD_LIBRARY_PATH="$PREFIX/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"

{
  echo "TerraVision source: $SOURCE"
  echo "Tcl/Tk prefix: $PREFIX"
  echo "Build ABI: i686-pc-linux-gnu (on x86_64 Debian with gcc -m32)"
  echo "Thread backend: POSIX pthreads selected by --with-pthreads"
  echo
  cd "$BUILD"
  "$SOURCE/configure" \
    --build=i686-pc-linux-gnu \
    --host=i686-pc-linux-gnu \
    --with-tcl="$PREFIX" \
    --with-pthreads \
    --disable-shared
  make -j"$JOBS"
} 2>&1 | tee "$LOG"

EXE="$BUILD/TclTV/TclTV"
if [[ -x "$EXE" ]]; then
  file "$EXE"
  file "$EXE" | grep -q 'ELF 32-bit' || {
    echo "ATTENZIONE: TclTV non risulta ELF 32-bit." >&2
    exit 1
  }
  echo "Build completata: $EXE"
else
  echo "Il make è terminato ma l'eseguibile atteso non c'è: controlla $LOG" >&2
  exit 1
fi
