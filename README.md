# TerraVision 6.0 — Original Source Recovery & Build

**Original SRI TerraVision · Linux i386 · Tcl/Tk 8.4 · OpenGL/GLX · NSPR/POSIX Threads**

A source-recovery, compatibility and build project dedicated to restoring the original TerraVision 6.0 terrain visualization software developed by SRI International.

The objective is to reproduce the original application as faithfully as possible, preserving its original C/C++ engine, Tcl/Tk interface, threading model, OpenGL rendering pipeline and data formats.

This is not a rewrite, a browser-based imitation or a replacement visualization engine.

---

## 1. Project objectives

The primary target is the original 32-bit Linux application:

- Original TerraVision 6.0 C/C++ engine.
- Original TclTV graphical interface.
- Tcl 8.4.20 and Tk 8.4.20 compatibility.
- Original NSPR runtime and POSIX pthread threading backend.
- Original OpenGL/GLX rendering.
- Original TSM data management and caching.
- Original GeoTIFF, TIFF, JPEG, PNG and ZLIB components.
- Original terrain, imagery, elevation and geospatial formats.
- Original configuration files, viewpoints and camera paths.

The project prioritizes source preservation, reproducible builds and reversible compatibility patches.

## 2. Supported platforms and recommended approaches

### A. Linux — primary platform

**Recommended baseline: Debian 12 Bookworm, x86_64, with i386 multilib.**

This is the preferred development and runtime environment.

The original program targets a 32-bit Linux ABI. A 64-bit Linux system with a working `gcc -m32` toolchain makes it possible to preserve that ABI without requiring an entirely 32-bit operating system.

Required components include:

- GCC and G++ multilib.
- 32-bit glibc development libraries.
- GNU Make and Binutils.
- Autoconf, Automake and Libtool.
- Tcl/Tk 8.4.20.
- X11 development libraries.
- OpenGL and GLU development libraries.
- Mesa software rendering as a fallback.
- Original third-party libraries.

Basic Debian preparation:

```bash
sudo dpkg --add-architecture i386
sudo apt update

sudo apt install \
  build-essential \
  gcc-multilib \
  g++-multilib \
  libc6-dev-i386 \
  autoconf \
  automake \
  libtool \
  libtool-bin \
  pkg-config \
  git \
  wget \
  curl \
  perl \
  python3 \
  xorg-dev \
  libx11-dev \
  libxext-dev \
  libxi-dev \
  libxmu-dev \
  libxt-dev \
  libgl1-mesa-dev \
  libglu1-mesa-dev
```

The final 32-bit X11 and OpenGL development libraries must also be installed for the enabled build modules.

Check the compiler:

```bash
gcc -m32 -v
g++ -m32 -v
```

Check actual executable support:

```bash
cat > /tmp/tv-test.c <<'EOF'
int main(void) { return 0; }
EOF

gcc -m32 /tmp/tv-test.c -o /tmp/tv-test

file /tmp/tv-test
/tmp/tv-test
```

Expected architecture:

`ELF 32-bit LSB pie executable, Intel 80386`

Build Tcl/Tk and then TerraVision using the repository build scripts.

The Linux desktop should provide X11 and a compatible OpenGL implementation.

**For best fidelity, run the original application directly inside the Linux desktop rather than replacing its graphics engine.**

### B. Linux 32-bit systems

A native i386 Linux installation is another possible target.

Advantages:

- Native 32-bit ABI.
- No `-m32` multilib setup.
- Historically compatible architecture.

Disadvantages:

- Reduced availability of modern distributions and development packages.
- Older 32-bit systems may have outdated security and graphics stacks.

This route is useful for historical compatibility testing but is not the preferred general-purpose development environment.

### C. Linux containers — Docker / Podman

A reproducible Linux build environment can also be provided using Docker or Podman.

Recommended architecture:

- Debian 12 x86_64 container.
- i386 multilib toolchain.
- Pinned build dependencies.
- Persistent source and build directories.
- Explicit X11 or headless graphics configuration.

For a local Linux desktop, X11 forwarding can be used.

For a headless environment, use:

- Xvfb.
- Mesa software rendering.
- A lightweight desktop such as XFCE.
- noVNC and websockify for browser access.

The remote desktop must be authenticated and must not be exposed publicly without access control.

A container is particularly useful for repeatable builds and isolating historical dependencies.

### D. GitHub Codespaces — cloud development

GitHub Codespaces can provide a disposable Linux development environment directly from this repository.

Recommended configuration:

- x86_64 Linux.
- Debian-based development container.
- 2–4 CPU cores.
- 8–16 GB RAM when available.
- i386 multilib.
- Persistent Git repository.
- Private forwarded ports.

For graphical access, configure Xvfb, Mesa and an authenticated noVNC desktop.

GitHub Codespaces supports private port forwarding, allowing the repository owner to access remote services through an authenticated browser connection.

### E. macOS — Intel and Apple Silicon

**Native macOS execution is not currently supported.**

The original executable targets Linux ELF and the original graphics implementation uses Linux X11/GLX. A native macOS port would require additional work on the operating-system compatibility and graphics layers.

The recommended Mac strategy is to run the original application inside a Linux virtual machine.

#### Intel Mac — recommended configuration

For Intel Macs, including the MacBook Pro 2015 running macOS Monterey:

**UTM + Debian 12 amd64 + i386 multilib.**

Guest configuration:

- Architecture: x86_64.
- Guest operating system: Debian 12.
- CPU: 2–4 virtual cores, depending on host hardware.
- RAM: 4–8 GB.
- Disk: 40 GB or larger.
- Desktop: XFCE.
- Graphics: X11 and Mesa.

Use the amd64 Debian installation image, then configure the i386 compiler and libraries inside Debian.

The original TclTV binary remains 32-bit Linux. It is not converted into a native macOS executable.

Official resources:

- UTM: https://github.com/utmapp/UTM
- Debian: https://www.debian.org/
- Debian Bookworm installation: https://www.debian.org/releases/bookworm/

#### Apple Silicon Macs

An x86_64 Linux environment requires emulation or a compatible translation layer.

This is a different configuration from Intel Macs and may have additional performance and compatibility limitations.

UTM/QEMU is a possible route, but the original i386 ABI, OpenGL and graphics stack must be tested independently.

### F. Replit — experimental cloud execution

Replit is an alternative for users who do not want to install Linux locally.

The intended workflow is:

1. Connect GitHub to Replit.
2. Import the private TerraVision repository.
3. Ask Replit Agent to prepare the Linux development environment.
4. Configure the required i386 compiler and libraries.
5. Build the original Tcl/Tk runtime and TerraVision.
6. Configure Xvfb and Mesa.
7. Configure an authenticated noVNC desktop.
8. Access the original TclTV interface through the browser.

Repository:

https://github.com/fungousrex/TerraVision-OG-Monterey-Kit

Replit is an experimental target.

Importing a repository does not automatically guarantee compatibility with legacy 32-bit binaries, X11, GLX or desktop applications. These capabilities must be verified in the actual Replit environment before declaring it supported.

---

## 3. Original threading architecture

The original threading implementation must be preserved.

Important components include:

- NSPR.
- POSIX pthreads.
- Original `pthr` abstraction.
- Terrain and image cache workers.
- Background loading and data processing.

Relevant source files include:

- `ThreeDCore.c`
- `TsCache.c`
- `DotGeo.c`
- `tsmHost_web.c`

The build should use the original POSIX threading backend where supported.

The objective is to preserve the original concurrent loading and rendering architecture, not replace it with a single-threaded implementation.

---

## 4. Dependencies and historical compatibility

TerraVision 6.0 relies on several historical dependencies.

| Component | Purpose |
|---|---|
| Tcl 8.4.20 | Application scripting |
| Tk 8.4.20 | Original graphical interface |
| NSPR | Runtime and threading support |
| pthreads | POSIX concurrency |
| OpenGL | 3D rendering |
| GLX | X11/OpenGL integration |
| libjpeg | Image processing |
| zlib | Compression |
| libpng | PNG support |
| libtiff | TIFF support |
| libgeotiff | Geospatial TIFF |
| libungif | GIF support |
| TSM | Terrain and dataset management |

Legacy Autotools and Libtool compatibility may require patches.

All patches should be:

- Reversible.
- Documented.
- Kept separate from the original source whenever possible.
- Applied reproducibly by build scripts.

Do not silently replace original libraries with incompatible modern alternatives.

---

## 5. Terrain datasets and visualization resources

The original software uses its own terrain and imagery data formats.

Important formats include:

- `.oi`
- `.dem`
- `.map`
- `.model`

The repository includes original application source and build material, but this does not imply that all historical terrain datasets are included.

The original database references an older SRI data server. Its availability must be independently verified.

Long-term objectives include:

- Recover original datasets.
- Restore original viewpoints and camera paths.
- Rebuild missing data resources.
- Produce new compatible image-pyramid datasets.
- Produce compatible DEM pyramids.
- Test original dataset loading and level-of-detail management.

---

## 6. Build validation

A successful compilation must not be confused with a successful application launch.

The following validation stages are required:

1. Verify the i386 compiler and linker.
2. Compile and test Tcl/Tk 8.4.20.
3. Compile all original third-party dependencies.
4. Compile the original TerraVision core.
5. Compile TclTV.
6. Verify the resulting executable as ELF 32-bit.
7. Check runtime library dependencies.
8. Start an X11 display.
9. Verify OpenGL/GLX.
10. Launch the actual TclTV interface.
11. Load a compatible terrain dataset.
12. Verify camera navigation and terrain visualization.
13. Test concurrent loading and the original threading backend.

Example executable validation:

```bash
file build/TclTV/TclTV
ldd build/TclTV/TclTV
```

A successful build requires the actual TclTV executable.

A successful runtime test additionally requires the original graphical interface to open and operate correctly.

---

## 7. Current project status

**Status: SOURCE RECOVERED — BUILD IN PROGRESS**

The original TerraVision 6.0 source was reconstructed from the uploaded historical CVS/RCS repository.

Progress recorded during development:

- Original source archive recovered.
- 3,705 historical source files reconstructed.
- i386 compiler and linker verified in Linux.
- Tcl/Tk 8.4.20 successfully compiled.
- JPEG, ZLIB, PNG, TIFF and GeoTIFF build stages reached successfully.
- Legacy Autotools compatibility problems identified.
- libungif / Libtool recursive shell issue still requires resolution.

The complete original TclTV executable has not yet been verified.

The original OpenGL graphical interface has not yet passed a runtime test.

This repository must not be described as a completed or fully working port until those tests have passed.

---

## 8. Development priorities

The recommended implementation order is:

1. Stabilize the Debian 12 x86_64/i386 build.
2. Resolve all legacy Autotools and Libtool problems.
3. Complete original TclTV compilation.
4. Validate the original OpenGL/GLX graphics pipeline.
5. Verify original NSPR/pthreads concurrency.
6. Restore original geospatial datasets.
7. Establish reproducible Docker/Podman builds.
8. Prepare the UTM macOS environment.
9. Test GitHub Codespaces.
10. Evaluate Replit as an alternative remote desktop environment.

---

## 9. Project principles

- Preserve the original TerraVision implementation.
- Prefer the original libraries and algorithms.
- Keep the original 32-bit application ABI.
- Do not substitute a web viewer for the original engine.
- Do not claim successful compilation without build logs.
- Do not claim successful graphics support without a real rendering test.
- Preserve historical source files and licensing notices.
- Keep remote graphical sessions private and authenticated.
- Document every compatibility modification.

---

## 10. References

- Original repository: https://github.com/fungousrex/TerraVision-OG-Monterey-Kit
- Debian: https://www.debian.org/
- UTM: https://github.com/utmapp/UTM
- GitHub Codespaces: https://docs.github.com/en/codespaces
- Replit: https://replit.com/
- Replit Repos: https://replit.com/repos

**Final objective:** restore and run the real original TerraVision 6.0 application on modern Linux systems and make it accessible from macOS without rewriting its visualization engine.
