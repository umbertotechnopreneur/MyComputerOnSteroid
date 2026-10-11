# Validation record

## Initial bootstrap, 2026-10-11

Scope: the native COM provider scaffold, repository conventions and publication.
This record does not certify the planned launcher.

| Check | Result |
| --- | --- |
| Local Debug x64 compilation | Passed |
| Local Debug ARM64 cross-compilation | Passed |
| Compiler | MSVC 19.51.36260.0, Visual Studio 2026 |
| Windows SDK | 10.0.26100.0 |
| CMake | 4.3.1-msvc1 |
| Compiler warnings | `/W4 /WX` enabled; successful builds |
| Native DLL identity and exports | Both PE architectures, publisher metadata and two COM exports verified |
| Relative documentation links and source provenance | 35 relative links and all C++/resource attribution markers checked |
| Asset provenance | Approved logo copied byte-for-byte; SHA-256 matches the recorded source |
| GitHub build | Not yet observed |
| Automated tests | Not run; no suite exists yet |
| COM runtime activation and lifetime | Not run |
| Explorer registration and native UI | Not performed |
| ARM64 runtime | Not run |
| Human code review, security, accessibility and performance review | Not recorded |

Reproducible compilation commands:

```powershell
pwsh -NoProfile -File ./scripts/Build.ps1 -Architecture x64
pwsh -NoProfile -File ./scripts/Build.ps1 -Architecture arm64
```

The initial compilation exposed an unclosed namespace and a mismatch between
export exception specifications and the Windows SDK declarations. Both were
corrected before the successful builds above.

## Required integration evidence

Before describing the software as a working launcher, verify COM ownership,
activation, unload behavior and reversible registration in an explicitly chosen
environment. Then verify actual Explorer rendering, all four groups, each view
mode, background and item menus, mixed selection, drop and shortcut-only removal.

Keep Windows 11 compact-menu behavior, classic-menu fallback, wallpaper and
acrylic results separate. Test target preservation with disposable files. None
of these behaviors is established by DLL compilation or a conceptual mockup.
