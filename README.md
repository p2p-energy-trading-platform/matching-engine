# GridX Matching Engine

GridX Matching Engine is a high-performance, low-latency matching engine written in **C++20**. The project uses **CMake**, **Conan 2**, **LLVM Clang**, **Ninja**, and **CTest** for dependency management, builds, and testing.

## Prerequisites

Install the following tools before building the project:

* LLVM Clang
* clang-format
* clang-tidy
* CMake 3.24 or newer
* Conan 2.x
* Ninja

---

## Development Environment

The project is standardized on:

* **Compiler:** LLVM Clang
* **Build System:** CMake
* **Build Generator:** Ninja
* **Package Manager:** Conan 2
* **Test Framework:** GoogleTest
* **Test Runner:** CTest

---

## Installing Dependencies

### macOS

Install the required tools using Homebrew:

```bash
brew install llvm cmake ninja conan
```

Configure LLVM Clang as the default compiler:

```bash
export PATH="$(brew --prefix llvm)/bin:$PATH"
export CC=clang
export CXX=clang++
```

To make these settings permanent, add them to your shell configuration file (`~/.zshrc`, `~/.bashrc`, etc.).

Verify the installation:

```bash
clang++ --version
clang-format --version
clang-tidy --version
cmake --version
conan --version
ninja --version
```

---

### Ubuntu / Debian

```bash
sudo apt update

sudo apt install -y \
    clang \
    clang-format \
    clang-tidy \
    cmake \
    ninja-build \
    python3-pip

python3 -m pip install --user conan
```

Configure Clang:

```bash
export CC=clang
export CXX=clang++
```

---

### Fedora

```bash
sudo dnf install -y \
    clang \
    clang-tools-extra \
    cmake \
    ninja-build \
    python3-pip

python3 -m pip install --user conan
```

Configure Clang:

```bash
export CC=clang
export CXX=clang++
```

---

## Initial Project Setup

Run the following commands once after cloning the repository.

Generate the local Conan profile:

```bash
conan profile detect --force
```

Generate the Debug configuration:

```bash
conan install . \
    --build=missing \
    -s build_type=Debug \
    -c tools.cmake.cmaketoolchain:generator=Ninja
```

Generate the Release configuration:

```bash
conan install . \
    --build=missing \
    -s build_type=Release \
    -c tools.cmake.cmaketoolchain:generator=Ninja
```

> **Important**
>
> The project exposes the high-level CMake presets `debug` and `release`. These presets inherit from Conan-generated presets. Both Debug and Release `conan install` commands must be executed before using the project presets.

---

## Building

### Debug

```bash
cmake --preset debug
cmake --build --preset debug
```

### Release

```bash
cmake --preset release
cmake --build --preset release
```

---

## Running

Debug build:

```bash
./build/Debug/matching-engine
```

Release build:

```bash
./build/Release/matching-engine
```

---

## Testing

Run the Debug test suite:

```bash
ctest --preset debug --output-on-failure
```

Run the Release test suite:

```bash
ctest --preset release --output-on-failure
```

---

## Development Workflow

Typical development workflow:

```bash
# Configure once
conan install ...

# Configure CMake (only required after CMake or Conan changes)
cmake --preset debug

# Build after source code changes
cmake --build --preset debug

# Run tests
ctest --preset debug

# Run the executable
./build/Debug/matching-engine
```

`conan install` only needs to be executed again when:

* Building the project for the first time
* The `build/` directory has been removed
* Dependencies in `conanfile.py` have changed
* Conan or toolchain configuration has changed

Editing `.cpp` or `.hpp` files does **not** require running `conan install` again.

---

## Project Structure

```text
matching-engine/
├── include/              # Public headers
├── src/                  # Source files
├── tests/
│   ├── unit/             # Unit tests
│   └── integration/      # Integration tests
├── build/                # Generated build files (ignored)
├── CMakeLists.txt
├── CMakePresets.json
├── conanfile.py
└── README.md
```

---

## Code Quality

The project uses:

* **clang-format** for source code formatting
* **clang-tidy** for static analysis
* **GoogleTest** for unit and integration testing
* **CTest** as the test runner

Compiler warnings are treated as errors using `-Werror`. All code should be formatted and pass static analysis before being committed.
