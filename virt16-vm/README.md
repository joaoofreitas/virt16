# Building the Virt16 VM

## Dependencies

**macOS**
```sh
brew install meson ninja glfw
```

**Linux (Debian/Ubuntu)**
```sh
sudo apt install meson ninja-build libglfw3-dev libgl1-mesa-dev libx11-dev
```

## Build

```sh
meson setup build
cd build && ninja
```

## Run

```sh
./build/virt16_vm
```

## Development

Code style is enforced via `.clang-format` at the repo root. Format all source files with:

```sh
find src -name "*.cpp" -o -name "*.h" | xargs clang-format -i
```

Key rules: Allman braces, `int* ptr` pointer alignment, 4-space indentation, no single-line `if`/`for`, 100 column limit.
