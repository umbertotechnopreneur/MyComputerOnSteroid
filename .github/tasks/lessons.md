# Engineering lessons

- DefView is a Shell view abstraction. Its internal window/control implementation
  is not a public customization contract.
- IExplorerCommand contributes commands; it does not display the entire modern
  Explorer context menu on an arbitrary HWND.
- The documented transparent/desktop folder flags apply to the desktop. DWM
  material support does not establish transparency for an embedded folder view.
- A favorite reference and its target have different deletion semantics. Enforce
  reference-only removal beneath the UI, including delegated context-menu verbs.
- A build, COM activation, registered Explorer session and visual check establish
  different facts. Keep their evidence separate.
- Use the current checked-in VibeWare templates: the movement identity and product
  publisher are separate, and historical template wording may have changed.
