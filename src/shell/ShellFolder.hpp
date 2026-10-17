// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; native launcher implementation; 2026-10-11.
 * Human guidance: Umberto Giacobbi; standard Shell view, native menus and MSI.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#pragma once
#include "../native/Support.hpp"
#include "../native/MenuIcons.hpp"
#include <atomic>

namespace mcos::shell
{
// DefView owns rendering; this adapter supplies data, grouping and interaction.
class ShellFolder final : public IShellFolder2, public IPersistFolder2,
    public IShellFolderViewCB, public IContextMenuCB, public IDropTarget, public IFolderViewSettings
{
public:
    ShellFolder() noexcept;
    IFACEMETHODIMP QueryInterface(REFIID, void**) noexcept override;
    IFACEMETHODIMP_(ULONG) AddRef() noexcept override;
    IFACEMETHODIMP_(ULONG) Release() noexcept override;
    IFACEMETHODIMP GetClassID(CLSID*) noexcept override;
    IFACEMETHODIMP Initialize(PCIDLIST_ABSOLUTE) noexcept override;
    IFACEMETHODIMP GetCurFolder(PIDLIST_ABSOLUTE*) noexcept override;
    IFACEMETHODIMP ParseDisplayName(HWND,IBindCtx*,PWSTR,ULONG*,PIDLIST_RELATIVE*,ULONG*) noexcept override;
    IFACEMETHODIMP EnumObjects(HWND,SHCONTF,IEnumIDList**) noexcept override;
    IFACEMETHODIMP BindToObject(PCUIDLIST_RELATIVE,IBindCtx*,REFIID,void**) noexcept override;
    IFACEMETHODIMP BindToStorage(PCUIDLIST_RELATIVE,IBindCtx*,REFIID,void**) noexcept override;
    IFACEMETHODIMP CompareIDs(LPARAM,PCUIDLIST_RELATIVE,PCUIDLIST_RELATIVE) noexcept override;
    IFACEMETHODIMP CreateViewObject(HWND,REFIID,void**) noexcept override;
    IFACEMETHODIMP GetAttributesOf(UINT,PCUITEMID_CHILD_ARRAY,SFGAOF*) noexcept override;
    IFACEMETHODIMP GetUIObjectOf(HWND,UINT,PCUITEMID_CHILD_ARRAY,REFIID,UINT*,void**) noexcept override;
    IFACEMETHODIMP GetDisplayNameOf(PCUITEMID_CHILD,SHGDNF,STRRET*) noexcept override;
    IFACEMETHODIMP SetNameOf(HWND,PCUITEMID_CHILD,PCWSTR,SHGDNF,PITEMID_CHILD*) noexcept override;
    IFACEMETHODIMP GetDefaultSearchGUID(GUID*) noexcept override;
    IFACEMETHODIMP EnumSearches(IEnumExtraSearch**) noexcept override;
    IFACEMETHODIMP GetDefaultColumn(DWORD,ULONG*,ULONG*) noexcept override;
    IFACEMETHODIMP GetDefaultColumnState(UINT,SHCOLSTATEF*) noexcept override;
    IFACEMETHODIMP GetDetailsEx(PCUITEMID_CHILD,const SHCOLUMNID*,VARIANT*) noexcept override;
    IFACEMETHODIMP GetDetailsOf(PCUITEMID_CHILD,UINT,SHELLDETAILS*) noexcept override;
    IFACEMETHODIMP MapColumnToSCID(UINT,SHCOLUMNID*) noexcept override;
    IFACEMETHODIMP MessageSFVCB(UINT,WPARAM,LPARAM) noexcept override;
    // Maps Explorer's relative menu offsets through the shared command table.
    IFACEMETHODIMP CallBack(IShellFolder*,HWND,IDataObject*,UINT,WPARAM,LPARAM) noexcept override;
    IFACEMETHODIMP DragEnter(IDataObject*,DWORD,POINTL,DWORD*) noexcept override;
    IFACEMETHODIMP DragOver(DWORD,POINTL,DWORD*) noexcept override;
    IFACEMETHODIMP DragLeave() noexcept override;
    // Folder-only drops start the manager asynchronously; other drops add owned
    // favorites. The negotiated effect never moves the original items.
    IFACEMETHODIMP Drop(IDataObject*,DWORD,POINTL,DWORD*) noexcept override;
    IFACEMETHODIMP GetColumnPropertyList(REFIID,void**) noexcept override;
    IFACEMETHODIMP GetGroupByProperty(PROPERTYKEY*,BOOL*) noexcept override;
    IFACEMETHODIMP GetViewMode(FOLDERLOGICALVIEWMODE*) noexcept override;
    IFACEMETHODIMP GetIconSize(UINT*) noexcept override;
    IFACEMETHODIMP GetFolderFlags(FOLDERFLAGS*,FOLDERFLAGS*) noexcept override;
    IFACEMETHODIMP GetSortColumns(SORTCOLUMN*,UINT,UINT*) noexcept override;
    IFACEMETHODIMP GetGroupSubsetCount(UINT*) noexcept override;
private:
    ~ShellFolder();
    std::atomic<ULONG> references_{1};
    Pidl root_;
    HWND view_window_{nullptr};
    bool acceptable_drop_{false};
    MenuIcons menu_icons_;
};
}
