# MyComputerOnSteroid

**Make This PC yours.**

I have many drives, folders and projects. I wanted my favorite places together.
So I built a small app inside File Explorer. Windows still draws the normal view.

Its name in Explorer is **My Computer Enhanced**.

Still in development. The new folder manager needs checks inside Explorer.
There is no published app release yet. [What I checked](docs/VALIDATION.md).

## Add your places

Open **My Computer Enhanced** under **This PC**.

Right-click its icon in **This PC** → **Show more options** →
**Configure My Computer Enhanced**. The same single command is available on empty
space inside the launcher. There is also a configuration shortcut in Start.
All settings share one native window: **This PC folders**, **Favorites**,
**Search and view**, **Recent items** and **About**.

- **Folder:** drag it onto empty space. It gets an entry directly in **This PC**.
- **App or file:** drag it onto empty space to add a favorite.
- **Name or icon:** open **Configure** → **This PC folders**. Choose an entry,
  edit it and press **Save changes**.
- **Remove:** choose an entry in that dialog and press **Remove**.

In the folder manager, **Add new** starts a new entry. Its main button reads
**Add** until you select an existing folder, then **Save changes**. The list shows
large icons and the editor previews the selected icon.

The dialogs follow the Windows display language: English, Italian, Vietnamese,
French, German, Spanish, Simplified Chinese or Hong Kong Cantonese. Other display
languages fall back to English. **About** links to my website and this repository.
The updated dialogs still need visual checks across monitors and display scales.

Windows asks for administrator permission when you change folders in This PC.
The real folders and files stay where they are.

## Make it tidy

Open **Configure** → **Search and view**.

Choose **Group by: Section**, **Order by: Name**, then **Apply**.
Use Explorer's **View** button to pick large or small icons.

You can also search your saved entries by name, path or file type.

## Recent files

Open **Configure** → **Recent items**, enable the section and press **Apply**.
You can pin a file to keep it, or hide it from this list.

## Install or build

Use Windows 11. Build and install the MSI for your PC: x64 or ARM64.
The Start-menu shortcut opens **My Computer Enhanced**.
The installer adds **Readme Setup.txt** as a file favorite for the installing
user, with configuration instructions. **Setup guide** in the configuration
window opens the same installed file. Removing its shortcut preserves the guide.

**Apply** saves search/view and recent preferences without closing the window.
**Close** discards unapplied preference edits. Folder and favorite action buttons
work immediately; they do not wait for Apply. Native folder/file pickers remain
ordinary Windows dialogs. The new tabbed window still needs visual acceptance.

[Build and install steps](docs/BUILD.md) ·
[How the code works](docs/ARCHITECTURE.md) ·
[Code cleanup examples](docs/CODE_REVIEW.md)

## My work

I'm **Umberto Giacobbi**. I created and maintain this project.
[My website](https://umbertogiacobbi.biz) ·
[LinkedIn](https://www.linkedin.com/in/umbertogiacobbi/)

More of my apps:
[WorkTrail](https://github.com/umbertotechnopreneur/WorkTrail),
[AgentInbox](https://github.com/umbertotechnopreneur/AgentInbox),
[PromptMeUp](https://github.com/umbertotechnopreneur/PromptMeUp).

[MIT license](LICENSE) · [Contribute](CONTRIBUTING.md) ·
[Get help](SUPPORT.md) · [Report a security issue](SECURITY.md)

---

<p align="right">
  <a href="https://umbertogiacobbi.biz/vibeware/manifesto">
    <img src="docs/assets/vibeware/vibeware-logo.png" alt="VibeWare movement logo" width="120">
  </a><br>
  Participating in the <strong>VibeWare movement</strong><br>
  <em>Human intent, AI execution, and plenty of tokens</em><br>
  <a href="https://umbertogiacobbi.biz/vibeware/manifesto"><strong>MANIFESTO</strong></a>
</p>

I develop this app with AI under my direction. Its creator and publisher is
Umberto Giacobbi. VibeWare is a movement I take part in.
