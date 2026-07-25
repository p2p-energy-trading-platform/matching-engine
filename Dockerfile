# Stage 1: Build
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install build tools
RUN apt-get update && apt-get install -y \
    build-essential \
    clang \
    cmake \
    ninja-build \
    python3 \
    python3-pip \
    python3-venv \
    git \
    && rm -rf /var/lib/apt/lists/*

# Create virtual environment
RUN python3 -m venv /opt/venv

# Add venv to PATH
ENV PATH="/opt/venv/bin:$PATH"

# Install Conan
RUN pip install --no-cache-dir --upgrade pip
RUN pip install --no-cache-dir conan

WORKDIR /app

# Copy project
COPY . .

# Detect compiler profile
RUN conan profile detect --force

# Install dependencies
RUN conan install . \
    -s build_type=Release \
    -c tools.cmake.cmaketoolchain:generator=Ninja \
    --build=missing

# Configure project
RUN cmake \
    -B build/Release \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release

# Build
RUN cmake --build build/Release

RUN ldd /app/build/Release/matching-engine

# Stage 2: Runtime

FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    ca-certificates \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY --from=builder \
    /app/build/Release/matching-engine \
    /app/matching-engine

ENTRYPOINT ["./matching-engine"]