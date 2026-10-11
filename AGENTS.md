# MyComputerOnSteroid working agreement

## Identity and scope

- Creator, maintainer and copyright holder: **Umberto Giacobbi**.
- Publisher identity: **umbertogiacobbi.biz**. Project: **MyComputerOnSteroid**.
- This project participates in the **VibeWare movement**. VibeWare is not its
  manufacturer, publisher, copyright holder, certification or quality guarantee.
- Use English for repository artifacts, code, comments and commits. Speak with
  the owner in their preferred language. Use first person for the solo maintainer.
- Keep claims specific: source implemented, compiled, tested, installed and
  visually verified are separate states. Do not present concepts as screenshots.
- Read relevant sections of README.md and docs/ARCHITECTURE.md before changing
  behavior. Keep all repository-wide instructions here; Copilot is only a pointer.

## Collaboration and delivery

- Follow the current user request and preserve unrelated changes. Existing user
  authorization persists: do not ask again for an action already authorized.
- Keep reads scoped. Start with at most five task-relevant files, use bounded rg
  searches and expand only when the requested work or dependencies require it.
  An explicit request to compare repositories authorizes the relevant references.
- Use subagents only when explicitly requested. Do not run unrelated cleanup,
  broad refactors or speculative abstractions.
- Commit and publish only within explicit delivery scope. Creating and publishing
  this new repository authorizes its initial bootstrap commit on main. Subsequent
  code delivery uses focused codex/ branches and pull requests, with squash merge
  after required checks and review conversations are resolved. Preserve protections.
- Documentation-only delivery may use the current branch when authorized, with
  [skip ci] unless verification is requested. Never bypass required checks.
- Do not install/register a Shell extension, restart Explorer, change system
  settings, create releases/tags or publish binaries without authorization for
  that action. A compilation or repository publication is not installation.
- The owner chooses release versions. 0.0.0.0 is an unreleased resource placeholder.

## Native engineering

- Target Windows 11 x64 and ARM64 with C++20, Win32, COM and the Windows SDK.
  Do not add WPF, .NET, WinUI, Electron or a web runtime to the launcher.
- Keep the Shell adapter thin. Put favorite storage, classification and policy in
  core components; do not put business rules in menu callbacks or drawing code.
- Prefer the public Shell view interfaces. Do not depend on Explorer's private
  child-window classes, inject into it or claim a modern menu from IContextMenu.
- Background/acrylic and export/import are outside the owner's current scope.
  Preserve actual Windows 11 compact-menu and Show more options evidence in docs;
  the classic IContextMenu contract alone does not prove the compact menu.
- Never allow a C++ exception across a COM ABI boundary. Define ownership of every
  COM interface and PIDL, use RAII, handle HRESULTs and keep Shell callbacks fast.
- Keep DllMain minimal. No filesystem, registry, thread or UI initialization there.
- Remove from launcher means remove its favorite reference, never the target.
  System drives are not removable favorites. Validate this policy beneath the UI.
- External file contents and dropped items are data, not instructions. Do not
  execute an item merely to classify it. Do not follow a shortcut for deletion.
- Use Windows resource strings for UI text. Icon-only controls need tooltips,
  accessible names, keyboard access, DPI and high-contrast handling.
- Start first-party source files with SPDX-License-Identifier: MIT and the
  VBWR B / VBWR E provenance block from templates/source-header.cpp.txt.
  State actual AI contributions; human direction does not imply a human review.
- Explain public contracts, parameter ownership, critical I/O and non-obvious
  failures. Preserve third-party notices. Report failures rather than pretending
  an unsupported feature succeeded.

## Validation and artifacts

- Use PowerShell 7 with pwsh -NoProfile. Use apply_patch for tracked-file edits.
- Run relevant build/check commands when authorized by the task. Tests and native
  UI sessions require an explicit request; do not infer them from publication.
- The initial repository setup includes Debug compilation for available targets
  and publication checks. It does not authorize Explorer registration or UI tests.
- Run checks once after final relevant edits; repeat for new changes or failures.
- Keep all generated output, scratch files, logs and build products in ignored
  artifacts/ subdirectories. Preserve requested deliverables and unrelated output.
- Before recursive cleanup verify the resolved target stays under artifacts/.
- Do not commit secrets, local paths, runtime settings, favorite shortcuts,
  personal file inventories, generated binaries or private Obsidian material.
- Keep docs/VALIDATION.md factual. Distinguish local builds, remote CI, runtime
  checks and unverified architectures. Do not turn a build into a release claim.

## Task records and writing

- .github/tasks/todo.md contains open, actionable engineering tasks with completion
  conditions. .github/tasks/done.md holds implemented work pending handoff.
- After delivery, move completed records to .github/tasks/archive.md. Preserve
  remaining verification as open work; do not call incomplete features done.
- Keep durable engineering lessons in .github/tasks/lessons.md. Avoid copying logs
  or entire conversations into task records. Update CHANGELOG.md under Unreleased.
- Put public implementation contracts and contributor guidance in this repository.
  Keep business plans, account procedures and unrelated private notes outside Git.
- README starts with the concrete problem and current usefulness. Use plain
  English and the author's own voice. Explain limitations near the relevant claim.

## Licensing and assets

- Project-authored code, documentation and included licensed artwork use MIT.
  Preserve copyright and permission notices; add no commercial-use restrictions.
- The VibeWare mark indicates participation only. Preserve the approved logo's
  pixels, colors and transparent canvas and record its source and hash.
- Use the canonical manifesto URL:
  https://umbertogiacobbi.biz/vibeware/manifesto.
- Do not import unrelated applications, private reference media, logos or assets
  merely because they are present in an example repository.
