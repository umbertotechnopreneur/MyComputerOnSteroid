# SPDX-License-Identifier: MIT
<# VBWR B
Project: MyComputerOnSteroid
Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
VibeWare participation: Human intent, AI execution, and plenty of tokens
Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
Created with AI: OpenAI Codex; native MSI build entry point; 2026-10-11.
Human guidance: Umberto Giacobbi; native C++, standard Windows installer.
Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
Copyright (c) 2026 Umberto Giacobbi
License: MIT
VBWR E #>
#Requires -Version 7.0
[CmdletBinding()]
param(
    [ValidateSet('x64','arm64')][string] $Architecture = 'x64',
    [ValidatePattern('^\d+\.\d+\.\d+$')][string] $Version = '0.0.0',
    # Reuse the installed ProductCode only for an explicit development repair.
    [ValidatePattern('^(\*|\{[0-9A-Fa-f-]{36}\})$')][string] $ProductId = '*',
    [string] $WixDirectory = (Join-Path ([Environment]::GetFolderPath('ProgramFilesX86')) 'WiX Toolset v3.14/bin'),
    [switch] $SkipBuild
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$projectRoot = Split-Path -Parent $PSScriptRoot
$candle = Join-Path $WixDirectory 'candle.exe'
$light = Join-Path $WixDirectory 'light.exe'
if (-not (Test-Path -LiteralPath $candle) -or -not (Test-Path -LiteralPath $light)) {
    throw 'Install WiX Toolset 3.14, or pass -WixDirectory containing candle.exe and light.exe.'
}
if (-not $SkipBuild) { & "$PSScriptRoot/Build.ps1" -Architecture $Architecture -Configuration Release }
$bin = Join-Path $projectRoot "artifacts/build/windows-$Architecture-release/bin"
$output = Join-Path $projectRoot "artifacts/installer/$Architecture"
New-Item -ItemType Directory -Path $output -Force | Out-Null
$object = Join-Path $output 'Product.wixobj'
$package = Join-Path $output "MyComputerOnSteroid-$Version-$Architecture.msi"
$native = if ($Architecture -eq 'x64') { 'AMD64' } else { 'ARM64' }
& $candle -nologo -arch $Architecture "-dArchitecture=$Architecture" "-dNativeArchitecture=$native" "-dProductVersion=$Version" "-dProductId=$ProductId" "-dProjectRoot=$projectRoot" "-dBinDir=$bin" -out $object "$projectRoot/installer/Product.wxs"
if ($LASTEXITCODE -ne 0) { throw "MSI authoring failed ($LASTEXITCODE)." }
& $light -nologo -out $package $object
if ($LASTEXITCODE -ne 0) { throw "MSI linking or validation failed ($LASTEXITCODE)." }
Get-FileHash -LiteralPath $package -Algorithm SHA256
Write-Host "MSI: $package"
Write-Host '0.0.0 is an unreleased development package. No installation or publication was performed.'
