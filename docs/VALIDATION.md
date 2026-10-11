# Validation record

## Group order and document transfer, 2026-10-12

- Local x64 Debug and final Release native suites passed with `/W4 /WX`; final
  Release covers real document CF_HDROP paths, renamed aliases, recent cache
  hashes, mixed-section selections, format enumeration and copy-only attributes.
- The new transfer check run against the previously installed DLL reproduced an
  unavailable CF_HDROP format in its mixed-section data object. The updated DLL
  passes the same check and preserves reference identities and original bytes.
- Category tests cover stable IDs, Drives/Folders/Apps ordering, app grouping,
  Office before PDF regardless of family-definition order, Development before
  Other extensions, and unknown category rejection. Name/date ordering remains
  independent. Final Debug ARM64 cross-compilation passed; no ARM64 runtime claim.
- x64 Release MSI passed WiX ICE validation. Authorized repair returned 3010;
  Explorer restart loaded the Program Files DLL matching the tested Release hash.
- A read-only probe obtained IDataObject from the actual Explorer folder and
  verified both exported document paths against their shortcut targets. Native
  UI Automation and a private screenshot showed Drives, Folders, Office and PDF
  in that order; Apps was empty in that collection. Native layout was preserved.
- No document was uploaded to GPT. Actual GPT/other receivers and an end-to-end
  desktop drag remain acceptance checks; COM data-format checks do not prove them.
- InfoTips were discussed only; no custom tooltip implementation was added.
  Windows file associations and icons remain unchanged by this work.

## Collection organization and dialog polish, 2026-10-11

This extends the installed x64 prototype with collection search, extension/family
groups and name/date ordering. It does not close the unrelated acceptance tasks
below or establish a release.

| Check | Result |
| --- | --- |
| Final Debug/Release x64 compilation and isolated native suite | Passed with `/W4 /WX` |
| Final Debug/Release ARM64 cross-compilation | Passed; ARM64 runtime not tested |
| Both native Release MSIs | Passed full WiX ICE validation |
| Installed payload | Matches tested Release DLL; authorized Explorer restart loaded Program Files DLL |
| Native search/options dialog | Invoked from the installed provider in actual Windows 11 Explorer; Apply returned S_OK |
| Dialog presentation | Right-aligned labels, group boxes, readable hints and colored OS stock icons visually checked at current desktop scale |
| Actual view search | Name, target path and case/dot-normalized exact extension filters passed; view count converged to filtered enumeration |
| Empty search results and recovery | No-match collection empty; clearing query restored the original collection |
| Actual view grouping | Extension and family keys applied; custom PDF/Markdown family header visibly rendered after bringing a matching item into view |
| Actual view order | Name ascending and typed Item date descending with name tie-breaker verified through active IFolderView2 |
| Reopening | Extension grouping and name ordering retained after Explorer restart |
| Details properties | Original five visible columns retained; eight total available |
| Realized labels after updates | No empty or serialized hexadecimal labels in the sampled native items |
| Broader DPI/themes/accessibility | Not tested; remaining acceptance stays open |

The native suite covers target suffix versus alias/.lnk, name versus path scope,
exact extension matching, editable families and duplicate rejection, Unicode
atomic persistence, existing categorizer updates, actual filtered COM enumeration,
typed chronological dates, stable tie-breaks, corrupt-preference fallback,
original-byte preservation and COM unload. Tests use isolated stores, not private
Windows history.

Explorer refresh is asynchronous: an intermediate empty view was observed after
clearing a no-match filter. Bounded checks waited for its item count to converge
to enumeration before asserting recovery. A custom header below the viewport was
not initially realized by UI Automation; bringing a matching item into view
confirmed the header visually and through UIA. These diagnostics did not require
product workarounds or private window classes.

Actual UI experiments temporarily changed search/group/order and a family
definition, then restored the prior preferences through the native dialog. No
favorites or originals were deleted. Private inventory and screenshots remain in
ignored artifacts and are not published. File icons still use Windows associations;
no external C/C++/MP3/WAV icon pack was imported.

The matching 0.0.0 x64 MSI repair returned 3010. No OS reboot was forced. Installed
and tested Release DLL SHA-256:
`AD7863B600695289086D7FCBE71FDAD1EE5A908708610655EC4F63BAF0CDDB4D`.
The old per-user registration was not removed. Required source publication checks
remain Debug x64 (including isolated tests) and Debug arm64; their current results
are recorded on the delivery PR and GitHub Actions.

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
unloaded. The final repair, including the older-font Remove fallback, returned
3010; its installed DLL matched the tested Release build, SHA-256
`2F95ECA7D719160E6F4E37ED1D20F89E30E0209920CFB5ED0EF5878FDF603DFE`.

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
Debug x64 tests and both architecture builds passed in
[run 38151458395](https://github.com/umbertotechnopreneur/MyComputerOnSteroid/actions/runs/38151458395).
Source-delivery evidence is in the repository's PR checks and
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
