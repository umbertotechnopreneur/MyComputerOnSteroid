# SPDX-License-Identifier: MIT
<# VBWR B
Project: MyComputerOnSteroid
Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
VibeWare participation: Human intent, AI execution, and plenty of tokens
Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
Created with AI: OpenAI Codex; native build entry point; 2026-10-11.
Human guidance: Umberto Giacobbi; use installed C++ tools and keep output isolated.
Evidence: docs/VALIDATION.md and Git history.
Copyright (c) 2026 Umberto Giacobbi
License: MIT
VBWR E #>
#Requires -Version 7.0
[CmdletBinding()]
param(
    [ValidateSet('x64', 'arm64')]
    [string] $Architecture = 'x64'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
if (-not $IsWindows) { throw 'This project requires Windows and the Visual Studio C++ toolchain.' }

$projectRoot = Split-Path -Parent $PSScriptRoot
$vswherePath = Join-Path ([Environment]::GetFolderPath('ProgramFilesX86')) 'Microsoft Visual Studio/Installer/vswhere.exe'
if (-not (Test-Path -LiteralPath $vswherePath)) {
    throw 'Install Visual Studio or Build Tools with Desktop development with C++.'
}
$requiredComponents = @('Microsoft.VisualStudio.Component.VC.Tools.x86.x64')
if ($Architecture -eq 'arm64') {
    $requiredComponents += 'Microsoft.VisualStudio.Component.VC.Tools.ARM64'
}
$vsInstall = & $vswherePath -latest -prerelease -products '*' -requires $requiredComponents -property installationPath
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($vsInstall)) {
    throw "No Visual Studio installation has the required $Architecture C++ tools."
}

$hostArchitecture = switch ([System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture) {
    'X64' { 'amd64' }
    'Arm64' { 'arm64' }
    default { throw 'A 64-bit Windows development host is required.' }
}
$targetArchitecture = if ($Architecture -eq 'x64') { 'amd64' } else { 'arm64' }
$developerShell = Join-Path $vsInstall 'Common7/Tools/Launch-VsDevShell.ps1'
& $developerShell -Arch $targetArchitecture -HostArch $hostArchitecture -SkipAutomaticLocation

$cmakePath = Join-Path $vsInstall 'Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
$ninjaFolder = Join-Path $vsInstall 'Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja'
if (-not (Test-Path -LiteralPath $cmakePath)) {
    $cmakePath = (Get-Command cmake -ErrorAction Stop).Source
}
if (Test-Path -LiteralPath $ninjaFolder) { $env:PATH = "$ninjaFolder;$env:PATH" }

$preset = "windows-$Architecture-debug"
Push-Location -LiteralPath $projectRoot
try {
    & $cmakePath --preset $preset
    if ($LASTEXITCODE -ne 0) { throw "CMake configure failed ($LASTEXITCODE)." }
    & $cmakePath --build --preset $preset
    if ($LASTEXITCODE -ne 0) { throw "C++ compilation failed ($LASTEXITCODE)." }
    Write-Host "Built: artifacts/build/$preset/bin/MyComputerOnSteroid.Shell.dll"
    Write-Host 'Compilation only: no registration, Explorer restart or installation was performed.'
}
finally {
    Pop-Location
}
