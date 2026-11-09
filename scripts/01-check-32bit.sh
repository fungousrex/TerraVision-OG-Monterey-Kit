#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -m)" != "x86_64" ]]; then
  echo "ERRORE: la VM deve essere x86_64." >&2
  exit 2
fi

TMP="$(mktemp -d)"
trap 'rm -rf "$TMP"' EXIT
cat > "$TMP/probe.c" <<'C'
#include <stdio.h>
int main(void) { puts("TerraVision i386 toolchain: OK"); return 0; }
C
gcc -m32 "$TMP/probe.c" -o "$TMP/probe"
file "$TMP/probe"
"$TMP/probe"
file "$TMP/probe" | grep -q 'ELF 32-bit'
echo "OK: compilatore, linker e kernel eseguono programmi i386."
