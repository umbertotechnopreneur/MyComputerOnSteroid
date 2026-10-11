# SPDX-License-Identifier: MIT
<# VBWR B
Project: MyComputerOnSteroid
Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
VibeWare participation: Human intent, AI execution, and plenty of tokens
Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
Created with AI: OpenAI Codex; isolated developer registration; 2026-10-11.
Human guidance: Umberto Giacobbi; native C++, standard Windows installer.
Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
Copyright (c) 2026 Umberto Giacobbi
License: MIT
VBWR E #>
#Requires -Version 7.0
[CmdletBinding()]
param([string] $BinaryDirectory, [switch] $Unregister)
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$clsid = '{B36B9212-2D35-4E65-8A97-0AA5D280E157}'
$classPath = "HKCU:\Software\Classes\CLSID\$clsid"
$namespacePath = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\MyComputer\NameSpace\$clsid"
$marker = 'MyComputerOnSteroid.Development'
if (-not $Unregister -and (Test-Path -LiteralPath "HKLM:\Software\Classes\CLSID\$clsid")) {
    throw 'An installed machine registration already exists. Update the MSI instead of mixing development registrations.'
}
foreach ($path in @($classPath, $namespacePath)) {
    if (Test-Path -LiteralPath $path) {
        $owner = Get-ItemPropertyValue -LiteralPath $path -Name MCOSOwner -ErrorAction SilentlyContinue
        if ($owner -ne $marker) {
            throw "Registration is not owned by this development script: $path"
        }
    }
}
if ($Unregister) {
    foreach ($path in @($namespacePath, $classPath)) {
        # Exact project CLSID paths, verified above; never remove their parent keys.
        if (Test-Path -LiteralPath $path) { Remove-Item -LiteralPath $path -Recurse -Force }
    }
    Write-Host 'Development registration removed. Favorite data and binaries were preserved.'
    exit
}
if (-not $BinaryDirectory) { throw 'Supply the directory containing the compiled native binaries.' }
$source = (Resolve-Path -LiteralPath $BinaryDirectory).Path
foreach ($file in @('MyComputerOnSteroid.Shell.dll', 'MyComputerOnSteroid.exe')) {
    if (-not (Test-Path -LiteralPath (Join-Path $source $file))) { throw "Missing $file" }
}
# A fresh immutable directory avoids overwriting a DLL already loaded by Explorer.
$destination = Join-Path $env:LOCALAPPDATA ('MyComputerOnSteroid\Development\' + [guid]::NewGuid())
New-Item -ItemType Directory -Path $destination -Force | Out-Null
Copy-Item -LiteralPath (Join-Path $source 'MyComputerOnSteroid.Shell.dll'), (Join-Path $source 'MyComputerOnSteroid.exe') -Destination $destination
function Ensure-RegistrationKey([string] $Path) {
    # New-Item -Force clears existing registry subkeys; keep owned registrations intact.
    if (-not (Test-Path -LiteralPath $Path)) { New-Item -Path $Path | Out-Null }
    return Get-Item -LiteralPath $Path
}
foreach ($path in @($classPath)) {
    Ensure-RegistrationKey $path | Out-Null
    Set-Item -LiteralPath $path -Value 'My Computer Enhanced'
    New-ItemProperty -LiteralPath $path -Name MCOSOwner -Value $marker -PropertyType String -Force | Out-Null
}
$server = Ensure-RegistrationKey "$classPath\InprocServer32"
Set-Item -LiteralPath $server.PSPath -Value (Join-Path $destination 'MyComputerOnSteroid.Shell.dll')
New-ItemProperty -LiteralPath $server.PSPath -Name ThreadingModel -Value Apartment -PropertyType String -Force | Out-Null
$icon = Ensure-RegistrationKey "$classPath\DefaultIcon"
Set-Item -LiteralPath $icon.PSPath -Value "$env:SystemRoot\System32\imageres.dll,-109"
$folder = Ensure-RegistrationKey "$classPath\ShellFolder"
New-ItemProperty -LiteralPath $folder.PSPath -Name Attributes -Value ([int]::Parse('A8000110',[Globalization.NumberStyles]::HexNumber)) -PropertyType DWord -Force | Out-Null
New-ItemProperty -LiteralPath $classPath -Name System.IsPinnedToNameSpaceTree -Value 1 -PropertyType DWord -Force | Out-Null
Ensure-RegistrationKey $namespacePath | Out-Null
Set-Item -LiteralPath $namespacePath -Value 'My Computer Enhanced'
New-ItemProperty -LiteralPath $namespacePath -Name MCOSOwner -Value $marker -PropertyType String -Force | Out-Null
Write-Host "Registered current-user development build: $destination"
Write-Host 'Open MyComputerOnSteroid.exe to launch Explorer. Close this launcher view before changing builds.'
