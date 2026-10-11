# Build the native provider

## Prerequisites

- Windows 11 x64 or ARM64.
- PowerShell 7.
- Visual Studio 2022 or newer, or Build Tools, with Desktop development with C++.
- Windows SDK, MSVC C++ tools, CMake 3.25 or newer and Ninja.
- MSVC ARM64 tools for an ARM64 target.

The script discovers Visual Studio with `vswhere`, including an installed preview
edition, and enters its developer environment. It prefers the bundled CMake and
Ninja. No dependencies are downloaded or installed by the build.

From the repository root, run:

```powershell
pwsh -NoProfile -File ./scripts/Build.ps1 -Architecture x64
pwsh -NoProfile -File ./scripts/Build.ps1 -Architecture arm64
```

Run architecture builds sequentially. Each has an isolated directory:

- `artifacts/build/windows-x64-debug/`
- `artifacts/build/windows-arm64-debug/`

The DLL is in the selected directory's `bin/` folder. Debug builds use the MSVC
Debug runtime and are development artifacts, not redistributable releases.

## Visual Studio and CMake

Open the repository folder in Visual Studio and select the matching Debug preset.
The presets use an externally selected compiler architecture. The script is the
reproducible command-line entry point; a plain terminal needs the corresponding
Visual Studio developer environment before invoking CMake directly.

The project uses C++20, Unicode, `/W4`, `/WX`, `/permissive-`, `/sdl` and Control
Flow Guard. No managed runtime or third-party UI framework is linked.

## Build boundaries

This bootstrap exports a COM class factory, but has no registration procedure.
Do not use `regsvr32` on it: `DllRegisterServer` is not implemented. Compilation
does not change the registry, restart Explorer or open a window.

The workflow in `.github/workflows/build.yml` compiles Debug x64 and ARM64 on
Windows. It does not run runtime tests, register the DLL, publish a release or
upload binary artifacts. See [validation](VALIDATION.md) for observed results.

Keep generated files in ignored `artifacts/`. If removing a build directory,
first resolve it and verify that it is under this repository's `artifacts/`.
