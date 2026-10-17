# Engineering lessons

- Define merged menu actions, labels, glyphs and errors together. Numeric offsets
  in separate dispatch chains drift when a new command is inserted.
- Support only the current app format; do not retain importers or migration
  scripts for older standalone tools.
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
- A delegated target IPropertyStoreFactory can bypass the namespace's name/date
  properties. Let DefView build a store from the provider's GetDetailsEx instead;
  test IShellItem2 consumers as well as direct IShellFolder2 properties.
- Content/Tile views ask for layout property lists as well as names. Correct
  GetDisplayNameOf output alone did not prevent serialized property-store labels.
- A column proplist needs the documented 0 flag for columns visible by default.
  Unflagged entries exist in the property system but remain hidden in the view.
- Native menu bitmaps are borrowed by HMENU. Retain their GDI ownership until
  menus close, use premultiplied alpha, and preserve text when decoration fails.
- A successful asynchronous BrowseObject call is not proof that navigation
  finished. Confirm the active folder before querying or operating on its view.
- Apply collection preferences through public active-view interfaces and verify
  the root. Do not retain a Shell view inside its folder; that creates a COM cycle.
- A reference's alias, .lnk suffix, target extension and history-entry date are
  distinct metadata. Test each search/group/order field against its actual source.
- Refresh and filtering are asynchronous in Explorer. Wait for the view count to
  converge to enumeration before treating an intermediate empty view as failure.
- Namespace group identity and presentation order are different contracts. Use
  explicit category ranks without renumbering persisted section identifiers.
- Shell-aware destinations and file-upload receivers can consume different
  IDataObject formats. Validate CF_HDROP paths as well as Shell PIDL identity;
  a Shell-item round trip alone does not prove document-upload compatibility.
