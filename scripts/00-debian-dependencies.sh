#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname -m)" != "x86_64" ]]; then
  echo "ERRORE: questa procedura richiede una VM Debian x86_64 (uname -m = x86_64)." >&2
  echo "Su Apple Silicon usa UTM in modalità Emulate x86_64; su Intel usa Virtualize x86_64." >&2
  exit 2
fi

sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install -y --no-install-recommends \
  build-essential gcc-multilib g++-multilib libc6-dev-i386 \
  make perl autoconf automake libtool pkg-config curl ca-certificates \
  file patch bison flex xz-utils \
  xfce4 xorg dbus-x11 xauth xfonts-base mesa-utils spice-webdavd davfs2 \
  libx11-dev:i386 libxext-dev:i386 libxmu-dev:i386 libxi-dev:i386 \
  libxt-dev:i386 libsm-dev:i386 libice-dev:i386 libxpm-dev:i386 \
  libxft-dev:i386 libfontconfig1-dev:i386 libfreetype-dev:i386 \
  libxrender-dev:i386 libgl-dev:i386 libglu1-mesa-dev:i386 \
  libmotif-dev:i386 libglw1-mesa-dev:i386 \
  libx11-6:i386 libxext6:i386 libxmu6:i386 libxi6:i386 libxt6:i386 \
  libsm6:i386 libice6:i386 libxpm4:i386 libxft2:i386 \
  libfontconfig1:i386 libfreetype6:i386 libxrender1:i386 \
  libgl1:i386 libglu1-mesa:i386 libgl1-mesa-dri:i386 \
  libglx-mesa0:i386 libglw1-mesa:i386

cat <<'MSG'
Dipendenze installate.
Ora verifica l'ABI 32-bit con:
  bash scripts/01-check-32bit.sh
MSG
