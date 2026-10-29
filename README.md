# FileHunter

**Fast, local file search for Linux, Windows and macOS.**

FileHunter is a lightweight desktop application by **Copa Development**. It searches files recursively from a folder you choose and filters results by filename and extension.

## Features

- Recursive file search
- Filename filter (case-sensitive option)
- Extension filter (`pdf`, `png`, `cpp`, etc.)
- Results show path, size and last modified date
- Open files or containing folders
- Copy full file path
- Search runs in a worker thread so the interface stays responsive
- No account, cloud service or database required

## Tech stack

- C++20
- Qt 6 Widgets
- CMake 3.21+

## Build

### Arch Linux

```bash
sudo pacman -S --needed base-devel cmake ninja qt6-base
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/FileHunter
```

### Debian / Ubuntu

```bash
sudo apt update
sudo apt install build-essential cmake ninja-build qt6-base-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/FileHunter
```

### Fedora

```bash
sudo dnf install gcc-c++ cmake ninja-build qt6-qtbase-devel
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
./build/FileHunter
```

### Windows

Install:
- Visual Studio 2022 with **Desktop development with C++**
- Qt 6 (MSVC 2022 64-bit kit)
- CMake

Open **Developer PowerShell for VS 2022**, configure Qt's CMake package path if Qt is not already discoverable, then:

```powershell
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:\Qt\6.x.x\msvc2022_64"
cmake --build build --config Release
```

Run `build\Release\FileHunter.exe`.

### macOS

Install Xcode Command Line Tools and Qt 6. With Homebrew:

```bash
xcode-select --install
brew install cmake ninja qt
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
cmake --build build
./build/FileHunter
```

## Package / distribute

The app must be built separately for each target operating system and architecture. A Linux binary is not a Windows or macOS binary.

- **Linux:** package as AppImage, Flatpak, or native packages (`.deb`, `.rpm`, Arch package).
- **Windows:** deploy Qt dependencies with `windeployqt`, then package the folder as an installer or ZIP.
- **macOS:** deploy with `macdeployqt`, sign and notarize for public distribution.

Build and test on each target OS before publishing releases.

## Current limitations

- Searches the folder selected by the user; it does not index the entire computer in the background.
- Filesystem permissions may prevent access to some directories.
- Search is filename/extension based; file contents are not indexed.
- Large folders can take time to scan.

## License

MIT — see [LICENSE](LICENSE).

---

Made with C++ and Qt by **Copa Development**.
