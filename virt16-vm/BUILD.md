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
