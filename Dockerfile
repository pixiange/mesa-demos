# Dockerfile for building mesa-demos
#
# Build:
#   docker build -t mesa-demos .
#
# The built binaries will be installed under /usr/local inside the image.

FROM debian:bookworm-slim

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
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
    && pip3 install --break-system-packages meson \
    && rm -rf /var/lib/apt/lists/*

# Copy the source tree into the image
WORKDIR /src
COPY . .

# Configure and build
RUN meson setup _build \
    --prefix /usr/local \
    --buildtype release \
    -Dauto_features=enabled \
    -Dlibdecor-0:gtk=disabled \
    -Dlibdecor-0:demo=false \
    && ninja -C _build \
    && ninja -C _build install
