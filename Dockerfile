# Dockerfile for building mesa-demos
#
# This Dockerfile clones the mesa-demos source code from Git, builds it with
# Meson, and installs the binaries under /usr/local.
#
# Build:
#   docker build -t mesa-demos .
#
# Build a specific branch or tag:
#   docker build --build-arg MESA_DEMOS_BRANCH=mesa-demos-9.0.0 -t mesa-demos .
#
# Use a custom repository URL (e.g. a fork):
#   docker build --build-arg MESA_DEMOS_REPO=https://github.com/user/mesa-demos.git \
#     -t mesa-demos .
#
# If you are behind a firewall or have limited connectivity to the default
# Debian/PyPI servers, you can specify alternative mirrors:
#
#   docker build --network=host \
#     --build-arg APT_MIRROR=mirrors.aliyun.com \
#     --build-arg PYPI_INDEX=https://mirrors.aliyun.com/pypi/simple/ \
#     -t mesa-demos .
#
# Popular mirror options:
#   APT_MIRROR: mirrors.aliyun.com, mirrors.ustc.edu.cn,
#               mirrors.tuna.tsinghua.edu.cn
#   PYPI_INDEX: https://mirrors.aliyun.com/pypi/simple/
#               https://pypi.tuna.tsinghua.edu.cn/simple/
#               https://mirrors.ustc.edu.cn/pypi/web/simple/

FROM debian:bookworm-slim

# Optional: set to a Debian mirror hostname (e.g. mirrors.aliyun.com) to
# replace the default deb.debian.org sources.
ARG APT_MIRROR=
# Optional: set to an alternative PyPI index URL.
ARG PYPI_INDEX=
# Repository URL and branch/tag to clone
ARG MESA_DEMOS_REPO=https://gitlab.freedesktop.org/mesa/demos.git
ARG MESA_DEMOS_BRANCH=main

# Switch APT sources to the specified mirror (if provided)
RUN if [ -n "$APT_MIRROR" ]; then \
      sed -i "s|deb.debian.org|${APT_MIRROR}|g" /etc/apt/sources.list.d/debian.sources; \
    fi

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    pkg-config \
    ninja-build \
    python3-pip \
    git \
    glslang-tools \
    freeglut3-dev \
    libdecor-0-dev \
    libdrm-dev \
    libegl-dev \
    libgbm-dev \
    libgl-dev \
    libgles-dev \
    libglu1-mesa-dev \
    libosmesa6-dev \
    libpng-dev \
    libvulkan-dev \
    libwayland-dev \
    libx11-dev \
    libxcb1-dev \
    libxext-dev \
    libxkbcommon-dev \
    libxkbcommon-x11-dev \
    wayland-protocols \
    && pip3 install --break-system-packages \
       ${PYPI_INDEX:+--index-url "$PYPI_INDEX"} meson \
    && rm -rf /var/lib/apt/lists/*

# Clone the mesa-demos source code
WORKDIR /src
RUN git clone --depth 1 --branch "${MESA_DEMOS_BRANCH}" "${MESA_DEMOS_REPO}" mesa-demos

# Configure and build
WORKDIR /src/mesa-demos
RUN meson setup _build \
    --prefix /usr/local \
    --buildtype release \
    -Dauto_features=enabled \
    -Dlibdecor-0:gtk=disabled \
    -Dlibdecor-0:demo=false \
    && ninja -C _build \
    && ninja -C _build install
