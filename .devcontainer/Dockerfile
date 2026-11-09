FROM mcr.microsoft.com/devcontainers/base:bookworm

USER root
RUN dpkg --add-architecture i386 \
 && apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    build-essential gcc-multilib g++-multilib libc6-dev-i386 \
    make perl autoconf automake libtool pkg-config curl ca-certificates \
    file patch bison flex xz-utils gzip unzip \
    xfce4 xorg dbus-x11 xauth xfonts-base x11-utils mesa-utils xvfb x11vnc novnc websockify \
    libx11-dev:i386 libxext-dev:i386 libxmu-dev:i386 libxi-dev:i386 \
    libxt-dev:i386 libsm-dev:i386 libice-dev:i386 libxpm-dev:i386 \
    libxft-dev:i386 libfontconfig1-dev:i386 libfreetype-dev:i386 \
    libxrender-dev:i386 libgl-dev:i386 libglu1-mesa-dev:i386 \
    libmotif-dev:i386 libglw1-mesa-dev:i386 \
    libx11-6:i386 libxext6:i386 libxmu6:i386 libxi6:i386 libxt6:i386 \
    libsm6:i386 libice6:i386 libxpm4:i386 libxft2:i386 \
    libfontconfig1:i386 libfreetype6:i386 libxrender1:i386 \
    libgl1:i386 libglu1-mesa:i386 libgl1-mesa-dri:i386 \
    libglx-mesa0:i386 libglw1-mesa:i386 \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

USER vscode
WORKDIR /workspaces/TerraVision-OG-Monterey-Kit
ENV DISPLAY=:0
ENV LIBGL_ALWAYS_SOFTWARE=1
ENV TV_JOBS=4
