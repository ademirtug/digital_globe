param(
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Debug',
    [string]$WxRoot = $env:WXWIN,
    [ValidateSet('Auto', 'Bundled', 'System')][string]$WxProvider = 'Auto',
    [string]$BuildDirectory,
    [switch]$BootstrapIfMissing = $true
)
$ErrorActionPreference = 'Stop'
$projectDir = Split-Path $PSScriptRoot -Parent
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    throw 'Install CMake 3.24+ and Visual Studio C++ Build Tools, then rerun this command. A separate wxWidgets installation is not required.'
}
# The extension supplies BootstrapIfMissing:false to prohibit toolchain installers.
# wxWidgets source restoration is a project dependency build, not an OS installation.
if ($WxProvider -eq 'Auto') {
    $WxProvider = if ($WxRoot) { 'System' } else { 'Bundled' }
}
$dependencyOptions = @("-DDIGITAL_GLOBE_WX_PROVIDER=$($WxProvider.ToUpperInvariant())")
if ($WxProvider -eq 'System') {
    if (-not $WxRoot) { throw 'Pass -WxRoot or set WXWIN when using -WxProvider System.' }
    if (-not (Test-Path -LiteralPath (Join-Path $WxRoot 'include\wx\wx.h'))) { throw "wxWidgets headers were not found at $WxRoot. Use -WxProvider Bundled to download and build wxWidgets automatically." }
    $libraryDir = @('vc14x_x64_dll', 'vc_x64_dll') | ForEach-Object { Join-Path $WxRoot "lib\$_" } | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
    if ($env:WXWIDGETS_LIBRARY_DIR) { $libraryDir = $env:WXWIDGETS_LIBRARY_DIR }
    if (-not $libraryDir) { throw 'No x64 MSVC wxWidgets DLL libraries were found. Use -WxProvider Bundled for a project-local source build.' }
    $wxConfiguration = if ($Configuration -eq 'Debug') { 'mswud' } else { 'mswu' }
    $dependencyOptions += @("-DwxWidgets_ROOT_DIR=$WxRoot", "-DwxWidgets_LIB_DIR=$libraryDir", "-DwxWidgets_CONFIGURATION=$wxConfiguration")
    Write-Host "Using the installed wxWidgets SDK: $WxRoot"
} else {
    Write-Host 'Using project-local wxWidgets: first build downloads and compiles it; later builds reuse it.'
}
$buildDir = if ($BuildDirectory) { [System.IO.Path]::GetFullPath($BuildDirectory) } else { Join-Path $projectDir "build\$Configuration" }
$generatorOptions = @()
if (-not (Test-Path -LiteralPath (Join-Path $buildDir 'CMakeCache.txt'))) { $generatorOptions = @('-A', 'x64') }
& cmake -S $projectDir -B $buildDir @generatorOptions @dependencyOptions -DDIGITAL_GLOBE_BUILD_TESTS=ON
if ($LASTEXITCODE -ne 0) { throw 'CMake configuration failed. Check the compiler/CMake installation and network access for the first dependency download.' }
& cmake --build $buildDir --config $Configuration --parallel 4
if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
