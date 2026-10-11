# Build, test and install

## Prerequisites

- Windows 11 x64 or ARM64; PowerShell 7.
- Visual Studio 2022 or newer / Build Tools with Desktop development with C++.
- Windows SDK, MSVC, CMake 3.25+ and Ninja; ARM64 tools for that target.
- WiX Toolset 3.14 (candle.exe and light.exe) for MSI packaging.

Build.ps1 discovers Visual Studio through vswhere, including previews, and enters
its developer environment. It prefers bundled CMake/Ninja and downloads nothing.
Run invocations in separate PowerShell processes:

~~~powershell
pwsh -NoProfile -File ./scripts/Build.ps1 -Architecture x64 -Configuration Debug
pwsh -NoProfile -File ./scripts/Build.ps1 -Architecture x64 -Configuration Release
pwsh -NoProfile -File ./scripts/Build.ps1 -Architecture arm64 -Configuration Release
pwsh -NoProfile -File ./scripts/Test.ps1 -Architecture x64 -Configuration Release -SkipBuild
~~~

DLL, opener and test EXE are under artifacts/build/windows-<architecture>-<configuration>/bin/.
The project uses C++20, Unicode, static MSVC CRT, /W4 /WX, /permissive-, /sdl,
CFG, ASLR and NX. Debug output is for development. No managed UI runtime is linked.

Tests load the DLL without registering it. Disposable targets, history inputs
and separate stores are retained under artifacts/tests/. Run tests on matching
native Windows. These are not Explorer UI or MSI lifecycle checks.

## MSI

~~~powershell
pwsh -NoProfile -File ./scripts/Build-Msi.ps1 -Architecture x64
pwsh -NoProfile -File ./scripts/Build-Msi.ps1 -Architecture arm64
~~~

Packages are under artifacts/installer/<architecture>/. Full WiX ICE validation
runs during linking. Use -WixDirectory for the installed WiX 3.14 tools and
-SkipBuild to package existing Release output.

**0.0.0 is an unreleased development placeholder.** The owner chooses release
versions. Packaging does not install, sign, create a release or publish binaries.
The package uses Windows Installer's basic UI and no third-party UI/custom-action DLL.

Install the matching MSI to register My Computer Enhanced under This PC and the
navigation tree. Installation is per-machine and Windows requests elevation.
The Start-menu shortcut or MyComputerOnSteroid.exe opens the namespace.
A loaded provider can require an Explorer restart after an update; the installer
does not force one. Keep installer lifecycle and UI checks separate.

For an explicit development repair of the same installed 0.0.0 product, pass its
existing ProductCode through -ProductId. Reinstall with REINSTALL=ALL and
REINSTALLMODE=vamus to recache the newly authored MSI. Another ProductCode for the
same development version is not a supported update. Actual releases need a new
owner-chosen version.

Uninstall through Windows Installed Apps. The MSI owns its binaries, registration
and Start-menu shortcut. Per-user favorites, icon files and recent settings remain.
A clean-machine install/upgrade/uninstall round trip still needs validation.

## Developer registration

Register-Development.ps1 deploys an immutable Debug copy and registers only this
project's per-user CLSID/This PC junction. -Unregister is bounded by ownership
markers. The script refuses registration over an installed machine MSI.

On the tested workstation, the machine MSI provider opened successfully; an
earlier HKCU-only experiment was refused by Explorer. This does not establish
a general Windows restriction on per-user namespace registration.

Do not use regsvr32: no DllRegisterServer export is supplied.

## CI and artifacts

GitHub Actions compiles Debug x64 and ARM64 on Windows Server 2022. The x64 job
also runs the isolated suite. CI does not register, install, exercise Windows 11 UI
or publish binaries. Required PR checks are Debug x64 and Debug arm64.

Keep generated files, probes and private screenshots under ignored artifacts/.
Before recursive cleanup, resolve and verify the target is inside that directory.
See [validation](VALIDATION.md) for observed results.
