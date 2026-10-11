# SPDX-License-Identifier: MIT
<# VBWR B
Project: MyComputerOnSteroid
Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
VibeWare participation: Human intent, AI execution, and plenty of tokens
Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
Created with AI: OpenAI Codex; isolated native test runner; 2026-10-11.
Human guidance: Umberto Giacobbi; verify references without touching originals.
Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
Copyright (c) 2026 Umberto Giacobbi
License: MIT
VBWR E #>
#Requires -Version 7.0
[CmdletBinding()]
param(
    [ValidateSet('x64','arm64')][string] $Architecture = 'x64',
    [ValidateSet('Debug','Release')][string] $Configuration = 'Debug',
    [switch] $SkipBuild
)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
if (-not $IsWindows) { throw 'Native tests require Windows.' }
$nativeArchitecture = [System.Runtime.InteropServices.RuntimeInformation]::OSArchitecture.ToString().ToLowerInvariant()
if ($nativeArchitecture -ne $Architecture) { throw "Run $Architecture tests on native $Architecture Windows, not $nativeArchitecture." }
if (-not $SkipBuild) { & "$PSScriptRoot/Build.ps1" -Architecture $Architecture -Configuration $Configuration }
$projectRoot = Split-Path -Parent $PSScriptRoot
$bin = Join-Path $projectRoot "artifacts/build/windows-$Architecture-$($Configuration.ToLowerInvariant())/bin"
$tests = Join-Path $bin 'MyComputerOnSteroid.Tests.exe'
$dll = Join-Path $bin 'MyComputerOnSteroid.Shell.dll'
if (-not (Test-Path -LiteralPath $tests) -or -not (Test-Path -LiteralPath $dll)) { throw 'Build the matching DLL and native tests first.' }
$fixture = Join-Path $projectRoot ('artifacts/tests/' + [guid]::NewGuid().ToString('N'))
& $tests $dll $fixture
if ($LASTEXITCODE -ne 0) { throw "Native tests failed ($LASTEXITCODE). Fixtures retained at $fixture." }
Write-Host 'Native policy, COM and menu tests passed in an isolated store. No registration or UI session was performed.'
