#!/usr/bin/env bash
set -euo pipefail

WORK="${TV_WORK_DIR:-$HOME/TerraVision-OG}"
PREFIX="${TV_TCL_PREFIX:-$HOME/opt/terravision-tcl84}"
DOWNLOADS="$WORK/downloads"
DEPS="$WORK/deps"
mkdir -p "$DOWNLOADS" "$DEPS" "$PREFIX"

command -v gcc >/dev/null || { echo "gcc non trovato: esegui 00-debian-dependencies.sh" >&2; exit 1; }
"$(dirname "$0")/01-check-32bit.sh"

fetch() {
  local url="$1" out="$2"
  if [[ ! -s "$out" ]]; then
    curl --fail --location --retry 3 --retry-delay 2 "$url" -o "$out"
  fi
  gzip -t "$out"
}

fetch 'https://prdownloads.sourceforge.net/tcl/tcl8.4.20-src.tar.gz' "$DOWNLOADS/tcl8.4.20-src.tar.gz"
fetch 'https://prdownloads.sourceforge.net/tcl/tk8.4.20-src.tar.gz' "$DOWNLOADS/tk8.4.20-src.tar.gz"

[[ -d "$DEPS/tcl8.4.20" ]] || tar -xzf "$DOWNLOADS/tcl8.4.20-src.tar.gz" -C "$DEPS"
[[ -d "$DEPS/tk8.4.20" ]] || tar -xzf "$DOWNLOADS/tk8.4.20-src.tar.gz" -C "$DEPS"

JOBS="${TV_JOBS:-1}"

if [[ ! -f "$PREFIX/lib/tclConfig.sh" ]]; then
  echo "== Build Tcl 8.4.20 (32-bit) =="
  (
    cd "$DEPS/tcl8.4.20/unix"
    CC='gcc -m32' CFLAGS='-O2 -m32 -fcommon' LDFLAGS='-m32' \
      ./configure --prefix="$PREFIX" --enable-shared
    make -j"$JOBS"
    make install
  )
fi

if [[ ! -f "$PREFIX/lib/tkConfig.sh" ]]; then
  echo "== Build Tk 8.4.20 (32-bit, X11) =="
  (
    cd "$DEPS/tk8.4.20/unix"
    CC='gcc -m32' CFLAGS='-O2 -m32 -fcommon' \
      LDFLAGS="-m32 -L$PREFIX/lib -Wl,-rpath,$PREFIX/lib" \
      ./configure --prefix="$PREFIX" --with-tcl="$DEPS/tcl8.4.20/unix" --enable-shared
    make -j"$JOBS"
    make install
  )
fi

file "$PREFIX/lib"/libtcl8.4.so* "$PREFIX/lib"/libtk8.4.so* 2>/dev/null || true
printf '\nTcl/Tk 8.4.20 installato in: %s\n' "$PREFIX"
