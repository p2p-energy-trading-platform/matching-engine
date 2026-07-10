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

## Development Environment Setup

### macOS

Install the required tools:

```bash
brew install llvm cmake ninja conan
```

Configure LLVM Clang for the current terminal session:

```bash
export PATH="$(brew --prefix llvm)/bin:$PATH"
export CC=clang
export CXX=clang++
```

To make these settings permanent, add the commands above to your shell configuration file (for example, `~/.zshrc` or `~/.bashrc`) and reload your shell:

```bash
source ~/.zshrc
```

Verify the compiler:

```bash
which clang++
clang++ --version
```

The reported compiler should be the LLVM version installed by Homebrew.

---

### Ubuntu / Debian

Install the required tools:

```bash
sudo apt update

sudo apt install -y \
    clang \
    clang-tools \
    cmake \
    ninja-build \
    python3-pip

python3 -m pip install --user conan
```

Configure Clang for the current terminal session:

```bash
export CC=clang
export CXX=clang++
```

To make these settings permanent, add the commands above to your shell configuration file (for example, `~/.bashrc` or `~/.zshrc`) and reload your shell:

```bash
source ~/.bashrc
```

Verify the compiler:

```bash
which clang++
clang++ --version
```

### Fedora

Install the required tools:

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

To make these settings permanent, add the commands above to your shell configuration file and reload your shell.

Verify the compiler:

```bash
which clang++
clang++ --version
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

Ensure the corresponding Debug or Release Conan configuration has been generated during the initial project setup before using these presets.

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
conan install . \
    --build=missing \
    -s build_type=Debug \
    -c tools.cmake.cmaketoolchain:generator=Ninja

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

The project uses the following tools to maintain code quality:

* **clang-format** for source code formatting
* **clang-tidy** for static analysis
* **GoogleTest** for unit and integration testing
* **CTest** as the test runner

### Formatting

Format all source files:

```bash
cmake --build --preset debug --target format
```

Verify formatting without modifying files:

```bash
cmake --build --preset debug --target format-check
```

### Static Analysis

Run static analysis:

```bash
cmake --build --preset debug --target lint
```

> **Note**
>
> The `lint` target requires the project to be configured first so that CMake generates the `compile_commands.json` compilation database:
>
> ```bash
> cmake --preset debug
> ```

All code submitted to the repository should:

* Pass `format-check`
* Pass `lint`
* Compile without warnings (`-Werror`)
* Pass all unit and integration tests
