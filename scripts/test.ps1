param(
    [ValidateSet('Debug', 'Release')][string]$Configuration = 'Debug',
    [string]$WxRoot = $env:WXWIN,
    [ValidateSet('Auto', 'Bundled', 'System')][string]$WxProvider = 'Auto',
    [string]$BuildDirectory
)
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'build.ps1') -Configuration $Configuration -WxRoot $WxRoot -WxProvider $WxProvider -BuildDirectory $BuildDirectory
if ($LASTEXITCODE -ne 0) { throw 'Build failed.' }
$projectDir = Split-Path $PSScriptRoot -Parent
$buildDir = if ($BuildDirectory) { [System.IO.Path]::GetFullPath($BuildDirectory) } else { Join-Path $projectDir "build\$Configuration" }
& ctest --test-dir $buildDir -C $Configuration --output-on-failure
if ($LASTEXITCODE -ne 0) { throw 'Regression tests failed.' }
