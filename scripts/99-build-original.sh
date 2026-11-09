#!/usr/bin/env bash
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"

cat <<'MSG'
TerraVision 6.0 — build originale in Linux i386
Questo script installa le dipendenze e compila Tcl/Tk 8.4.20 + TerraVision.
Richiede una VM Debian x86_64 con rete e una sessione desktop Xfce.
MSG

bash "$HERE/00-debian-dependencies.sh"
bash "$HERE/01-check-32bit.sh"
bash "$HERE/02-build-tcltk84.sh"
bash "$HERE/03-build-original-terravision.sh"

cat <<'MSG'

BUILD TERMINATA.
Per lanciare la GUI originale TclTV, esegui:
  bash scripts/04-run-original-terravision.sh
Il log completo è in ~/TerraVision-OG/build-terravision.log
MSG
