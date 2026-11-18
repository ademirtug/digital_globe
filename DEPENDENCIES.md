# Digital Globe: build and distribution

The application uses C++20, wxWidgets (core, base and OpenGL canvas), OpenGL 3.3,
GLAD, GLM, stb_image and libcurl. wxWidgets owns the window, menus, preferences,
status bar and event loop. The renderer, camera mathematics and tile-selection
code are shared between Windows and Linux. GLFW is no longer linked.

Windows has been built and tested locally. The Linux build path is provided but
has not been built or run as part of this migration.

## Windows (x64)

Install **CMake 3.24+ and Visual Studio 2022/2026 C++ Build Tools**. A separate
wxWidgets installation, Git, .NET SDK or Perl installation is not required.
Then build from the repository root with one command:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -Configuration Release
```

Without `WXWIN` or `-WxRoot`, this downloads the official wxWidgets 3.3.4 source
archive, verifies its SHA-256, and compiles the required static libraries locally.
It also restores libcurl. The first build requires internet access and takes
longer; subsequent builds reuse downloaded sources and compiled dependencies.
Nothing is installed globally, and developer tools are not installed automatically.

wxWidgets is pinned to the official release archive with SHA-256
`a604fe40a59e6515eac3efe58deca38b54b7ef7f3c1df0906559bde6de1e2f58`.
libcurl 8.23.0 is downloaded from curl.se with SHA-256
`8dadd7d2536a314f982ad05a651b713f426bf7d02cc2c76cffb9796f2218e6c5`.
The Windows bundled build links wxWidgets, libcurl and the MSVC runtime statically;
HTTPS uses Schannel and Windows certificate validation. No wxWidgets DLLs or
separate Visual C++ runtime are required by this bundled build. System Windows
DLLs and an OpenGL 3.3 graphics driver are still required.

Outputs are `build/Release/bin/digital_globe.exe` and
`build/Debug/bin/digital_globe.exe`. Shaders and assets are copied alongside the
executable, which launches without a terminal window. Both configurations and the
regression suite use the same build script:

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/test.ps1 -Configuration Release
```

The existing Visual Studio solution delegates to CMake. VS Code has Windows/Linux
launch configurations, and the wxWidgets Tools extension recognizes this project.
The standard F5 build task can restore wxWidgets just like the command-line build.
The optional wxWidgets Tools extension may request its own installed SDK selection;
the command-line build does not require that extension or SDK.

### Optional installed SDK

An explicit `-WxRoot` or `WXWIN` selects an installed x64 MSVC DLL SDK instead.
The official installer suggests a versioned folder such as `C:\wxWidgets-3.3.4`,
but the build does not assume a global installation directory.
Both its import libraries and matching core/base/OpenGL DLLs must be present.
These DLLs are staged for the app and individual tests. This mode uses the matching
dynamic MSVC runtime, so it requires the corresponding Visual C++ runtime.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File scripts/build.ps1 -Configuration Release -WxRoot C:\wxWidgets-3.3.4
```

`-WxProvider Bundled` forces the downloaded static build even when `WXWIN` is set.
`-WxProvider System` requires an installed SDK. `-BuildDirectory` selects a separate
build folder, useful for verifying a build with no installed wxWidgets dependency.
The `BootstrapIfMissing` switch is retained for extension compatibility; source
dependency restoration is automatic even when toolchain installation is disabled.

## Linux (Ubuntu 24.04 or equivalent)

Use a C++20 toolchain with `std::format` and `std::jthread` support (GCC 13+),
wxWidgets 3.2+ with GL canvas support, OpenGL development libraries and libcurl
7.85+ development libraries. For Ubuntu:

```sh
sudo apt-get install build-essential cmake libwxgtk3.2-dev libwxgtk-gl3.2-dev libgl1-mesa-dev libcurl4-openssl-dev
bash scripts/build.sh Release
./build/Release/bin/digital_globe
```

Alternatively, `bash scripts/build.sh Release Bundled` downloads and statically
builds the pinned wxWidgets source. This still needs the system GTK development
packages (`libgtk-3-dev`), OpenGL and libcurl development packages; it does not run
a package manager automatically. The default Linux mode uses the system wxWidgets
package. Both Linux modes remain untested in this workspace.

`bash scripts/test.sh Debug` runs the regression suite in a graphical desktop
session. Tests create OpenGL windows and require a working display and GL driver.
No Docker or Linux execution was performed during this migration.

## Install and package

```sh
cmake --install build/Release --config Release --prefix dist
cpack --config build/Release/CPackConfig.cmake -C Release
```

Windows ZIP packages include the executable and resources. Installed-SDK builds
also include matching wxWidgets DLLs; bundled builds do not need those DLLs. Linux TGZ
packages use system wxWidgets/libcurl/OpenGL libraries and install a desktop entry.
Installed resources are resolved relative to the executable under `bin` or
`share/digital_globe`, so launching does not depend on the working directory.
Keep the license notices with redistributed binaries. Linux package generation
and installation have not been validated on a Linux host.

## Data and preferences

On Windows existing data remains compatible:
- Settings: `%LOCALAPPDATA%\digital_globe\settings.ini`
- Default tiles: `%LOCALAPPDATA%\digital_globe\mapdata`

On Linux:
- Settings: `$XDG_CONFIG_HOME/digital_globe/settings.ini` (default `~/.config/...`)
- Default tiles: `$XDG_CACHE_HOME/digital_globe/mapdata` (default `~/.cache/...`)

The cache has separate `osm`, `terrain` and `satellite` directories. Settings >
Preferences (F2) selects its location and limit (default 1,024 MB). Oldest tiles
are evicted at the limit. Zero clears managed tiles and disables disk caching.
Changing location copies downloaded tiles and retains the original files. Clear
cache asks for confirmation and only deletes managed tiles in the active cache.
Offline mode never requests new tiles. GL resource creation/destruction stays on
the UI thread; two background workers perform HTTPS and image decoding.

City maps use OpenStreetMap; Terrain uses Esri World Topographic Map; Satellite
uses Esri World Imagery. Source attribution is available in Help > Map sources
and credits. Terrain is a topographic basemap, not elevation geometry. The sky
image is credited in `assets/sky-source.md`. Map services have their own usage
terms; only viewed tiles are requested, with no bulk-download feature.

On wxWidgets 3.3+, VSync uses the canvas swap-interval API. On wxWidgets 3.2,
the same setting limits repaint requests to approximately 60 FPS; hardware
synchronization follows the graphics driver configuration.

GLM, stb_image, GLAD and ecs_s retain their license notices in their source files.
libcurl's COPYING and wxWidgets' license are included with Windows packages.
The following GLFW notice is retained for the legacy vendored headers only.

## GLFW notice

Copyright (c) 2002-2006 Marcus Geelnard

Copyright (c) 2006-2019 Camilla Löwy

This software is provided 'as-is', without any express or implied warranty. In no
event will the authors be held liable for any damages arising from the use of this
software.

Permission is granted to anyone to use this software for any purpose, including
commercial applications, and to alter it and redistribute it freely, subject to
the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that
   you wrote the original software. If you use this software in a product, an
   acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

Other vendored components retain their notices in their source headers and licence files;
copies are also included in the distribution's `licenses` folder.
