# Validation record

## Installed development prototype, 2026-10-11

Current scope: native launcher, optional recent documents, native menu glyphs and
development MSI. This is an installed x64 prototype with remaining acceptance
work, not a signed release. The visible node is **My Computer Enhanced**.

| Check | Result |
| --- | --- |
| Local Debug and Release x64 builds | Passed with `/W4 /WX` |
| Local Debug and Release ARM64 cross-builds | Passed with `/W4 /WX`; runtime not tested |
| Isolated x64 Debug and Release native suite | Passed; DLL loaded directly, separate disposable stores, no registration |
| Native x64 and ARM64 MSI authoring | Passed full WiX 3.14 ICE validation |
| x64 development MSI repair | Installed; installed DLL hash equals tested Release DLL |
| Actual Explorer host | Windows 11 x64, build 26300.9550, current-user Explorer session |
| Native Shell view and groups | Drives, folders, apps, files and optional recent documents observed in actual Explorer |
| Small icons, List, Details, Tiles, medium/large/extra-large icons and Content | Switched through public Shell interfaces; captures and realized-label checks passed |
| Recent labels | No empty/serialized-hex labels in the checked realized items; small, medium, Details, Tiles and Content visually inspected |
| Default Details columns | Five visible columns after installing and again in a newly opened Explorer window, without manual restoration |
| Windows 11 compact item menu | Observed on recent/favorite references, with native commands and Show more options |
| Show more options transition | Invoked in actual Explorer; classic recent menu showed all four launcher commands |
| Recent custom command icons | Star, pin, unpin and hide glyphs visibly rendered in the actual dark native popup |
| All custom icon surfaces | Native menu tests check present 32-bit bitmaps, visible glyph coverage, transparent pixels and premultiplied alpha |
| Asset provenance | Approved VibeWare logo unchanged; SHA-256 recorded in asset provenance |
| ARM64 runtime, light/high-contrast and multi-monitor DPI UI | Not tested |
| Full installer lifecycle, accessibility/security review | Open; see remaining checks below |

Compiler: MSVC 19.51.36260.0, Visual Studio 2026 Insiders; Windows SDK
10.0.26100.0; CMake 4.3.1-msvc1/Ninja. WiX 3.14.1.8722 packages native Release
payloads. ARM64 compilation/installer validation is separate from ARM64 execution.

Reproducible local checks:

~~~powershell
pwsh -NoProfile -File ./scripts/Build.ps1 -Architecture x64 -Configuration Debug
pwsh -NoProfile -File ./scripts/Test.ps1 -Architecture x64 -Configuration Debug -SkipBuild
pwsh -NoProfile -File ./scripts/Build.ps1 -Architecture x64 -Configuration Release
pwsh -NoProfile -File ./scripts/Test.ps1 -Architecture x64 -Configuration Release -SkipBuild
pwsh -NoProfile -File ./scripts/Build.ps1 -Architecture arm64 -Configuration Debug
pwsh -NoProfile -File ./scripts/Build.ps1 -Architecture arm64 -Configuration Release
pwsh -NoProfile -File ./scripts/Build-Msi.ps1 -Architecture x64 -SkipBuild
pwsh -NoProfile -File ./scripts/Build-Msi.ps1 -Architecture arm64 -SkipBuild
~~~

The suite verifies COM factory/lifetime, PIDL validation and ownership, enumeration,
groups, mixed-section data objects/menus, reference-only add/rename/remove, original
preservation, drive protection, ICO changes and preservation of shortcut arguments.
Recent tests cover opt-in behavior, source dates, duplicate/malformed/virtual input,
combined Unicode/category/extension filters, concurrent refresh, cached failure
fallback, bulk pin/unpin/hide/promotion, aging and remembered local removal.

Display regressions check friendly name aliases, IShellItem2 consumers, typed
Content/Tile property lists, property-factory boundaries, favorite aliases without
recent dates, and visible-by-default column flags. Menu tests compose actual
IContextMenu objects and invoke canonical/numeric commands on fixtures. They
are not proof of every Explorer UI gesture or modern-menu deletion behavior.

### What the display investigation established

Direct namespace names were already correct while native small/Content views
displayed serialized SPS1 property-store text. Target property-store factories
were bypassing launcher properties. The provider now declines those bindings and
lets DefView build its property store from GetDetailsEx. Explicit Content/Tile
layouts preserve the document name, target icon and recent-entry date. Favorites
use the same boundary so renamed launcher aliases retain their own labels.

The initially empty Details view was a separate issue. The column property list
omitted the documented visible-by-default flag. Adding 0 to each column, rather
than restoring columns manually, produced five default visible columns in a
freshly opened Explorer window.

UIA checks cover currently realized labels, not an exhaustive inventory of every
cached document. Actual screenshots and local history remain in ignored artifacts;
personal names, paths, shortcuts and inventories are not published.

### Installation and remaining checks

The matching x64 development product was repaired with REINSTALL=ALL and
REINSTALLMODE=vamus to recache the freshly authored MSI. Exit 3010 requested an
OS reboot for pending old-file cleanup. An authorized Explorer restart loaded the
new DLL; no OS reboot was forced. A subsequent repair after the session resumed
installed the font-compatibility update with exit 0, while the provider was
unloaded. The final installed/tested DLL SHA-256 was
`AA493D01565EBE468DA416E1FB8B00999EE8C7AE5FB0D6EB01AFA41163DC7A71`.

An old per-user development registration remains on this workstation. Its cleanup
was rejected by automatic approval review and was not bypassed. Actual Explorer
used the installed machine provider; stand-alone merged-registry activation can
still select the old development copy. Consequently a clean-machine registration
and install/upgrade/uninstall round trip remains open.

An earlier compact-menu Delete experiment removed a favorite row but left its
owned shortcut in the store. Persistent deletion through that UI path remains
unverified. Isolated core/classic command tests do remove the owned reference and
preserve its original. Do not turn those tests into a claim about compact Delete.

Also open: exact mixed UI selections, actual Ctrl/Shift recent operations, combined
dialog input, date sorting/collapse acceptance, icon-picker last-directory UI,
light/high-contrast and monitor DPI changes, native ARM64 runtime and an independent
source/security/accessibility review. The owner deferred further recent-management
UX work. Background/acrylic, its photo control and export/import were cancelled.

The current workflow adds isolated x64 tests to Debug x64/ARM64 compilation.
Its first x64 run exposed an absent Hide bitmap with the runner's icon fonts;
Diagnostic output confirmed Fluent font substitution and a missing Hide glyph in
MDL2. The renderer tries the documented Remove symbol in either font if needed. The
Windows 11 Fluent rendering observed above retains the eye-with-slash glyph.
Required PR checks remain **Debug x64** and **Debug arm64**. Current remote
source-delivery evidence is in the repository's PR checks and
[GitHub Actions](https://github.com/umbertotechnopreneur/MyComputerOnSteroid/actions);
the fixed historical run below proves the bootstrap only.

## Initial bootstrap, 2026-10-11 (historical)

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
| Relative documentation links and source provenance | Local links and all C++/resource attribution markers checked |
| Asset provenance | Approved logo copied byte-for-byte; SHA-256 matches the recorded source |
| GitHub build | Debug x64 and ARM64 passed on Windows Server 2022 runners |
| GitHub publication | Public repository; GitHub recognizes the MIT license |
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

Remote evidence: [Native build run 38127650216](https://github.com/umbertotechnopreneur/MyComputerOnSteroid/actions/runs/38127650216)
for source commit `ffa56301b1dc61ce1d38ff58838a7034eeb0f437`.
Both architecture jobs completed successfully. Subsequent delivery-record edits
change documentation only and do not imply additional runtime verification.

The installed prototype evidence above supersedes the historical runtime gaps
only where a specific check is recorded. Remaining acceptance stays open in
[the task list](../.github/tasks/todo.md).
