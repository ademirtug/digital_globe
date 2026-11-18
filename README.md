# Digital Globe

## Introduction

Digital Globe is a desktop map viewer for exploring the world on an interactive
3D globe. Switch between city maps, topographic terrain maps, and satellite imagery,
with downloaded tiles cached for offline viewing.

Drag to pan and use the mouse wheel to zoom. Open **Settings > Preferences (F2)**
to change the map style, coordinate format, cache location and size limit, or enable
offline mode. Press **F5** to reset the view.

## Installation

Download or clone this repository, then follow the instructions for your platform
from the project folder. A graphics driver supporting **OpenGL 3.3** is required.

### Windows (x64)

Install [CMake 3.24 or later](https://cmake.org/download/) and
[Visual Studio 2022 or 2026 C++ Build Tools](https://visualstudio.microsoft.com/visual-cpp-build-tools/)
with the **Desktop development with C++** workload and a Windows SDK. Build and run:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -Configuration Release
.\build\Release\bin\digital_globe.exe
```

The build automatically downloads and compiles wxWidgets and libcurl unless an
installed wxWidgets SDK is selected through `WXWIN` or `-WxRoot`. No separate
wxWidgets installation is needed. The first build requires internet access;
subsequent builds reuse the downloaded dependencies.

### Linux

Install GCC 13 or later, CMake 3.24 or later, and the wxGTK, OpenGL, and libcurl
development packages. See the [setup guide](DEPENDENCIES.md) for Ubuntu package
names and the optional bundled wxWidgets build. Then build and run:

```sh
bash scripts/build.sh Release
./build/Release/bin/digital_globe
```

Windows builds are tested. Linux build support is provided but has not yet been
validated on a Linux host.

## Documentation

See the [build and distribution guide](DEPENDENCIES.md) for dependency options,
tests, packaging, and settings and cache locations. Keep the `assets` and `shaders`
folders with the executable when copying a build.

Offline mode displays previously downloaded tiles for the selected map style.
New areas require an internet connection. Terrain maps provide topographic imagery;
they do not change the globe's elevation geometry. Map credits are available under
**Help > Map sources and credits**.

## Reporting Issues

Report bugs and request features in the
[GitHub issue tracker](https://github.com/ademirtug/digital_globe/issues).
Include your operating system, app version or commit, steps to reproduce, and any
error message or build output.

## License

Digital Globe is released under the [MIT License](LICENSE). Third-party libraries,
map services, and imagery retain their respective licenses and terms; see the
[dependency guide](DEPENDENCIES.md) and [sky image credits](assets/sky-source.md).
