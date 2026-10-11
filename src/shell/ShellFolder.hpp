// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; Shell folder interface declarations; 2026-10-11.
 * Human guidance: Umberto Giacobbi; native C++, explicit validation evidence.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#pragma once

#include <ShlObj.h>
#include <atomic>

namespace mcos::shell
{
// Bootstrap provider: exposes an empty namespace and the system's default view.
// It does not advertise favorite, drive, drop, deletion or menu capabilities.
class ShellFolder final : public IShellFolder2, public IPersistFolder2
{
public:
    ShellFolder() noexcept;

    IFACEMETHODIMP QueryInterface(REFIID iid, void** result) noexcept override;
    IFACEMETHODIMP_(ULONG) AddRef() noexcept override;
    IFACEMETHODIMP_(ULONG) Release() noexcept override;

    IFACEMETHODIMP GetClassID(CLSID* class_id) noexcept override;
    IFACEMETHODIMP Initialize(PCIDLIST_ABSOLUTE pidl) noexcept override;
    IFACEMETHODIMP GetCurFolder(PIDLIST_ABSOLUTE* pidl) noexcept override;

    IFACEMETHODIMP ParseDisplayName(HWND, IBindCtx*, PWSTR, ULONG*, PIDLIST_RELATIVE*, ULONG*) noexcept override;
    IFACEMETHODIMP EnumObjects(HWND, SHCONTF, IEnumIDList**) noexcept override;
    IFACEMETHODIMP BindToObject(PCUIDLIST_RELATIVE, IBindCtx*, REFIID, void**) noexcept override;
    IFACEMETHODIMP BindToStorage(PCUIDLIST_RELATIVE, IBindCtx*, REFIID, void**) noexcept override;
    IFACEMETHODIMP CompareIDs(LPARAM, PCUIDLIST_RELATIVE, PCUIDLIST_RELATIVE) noexcept override;
    IFACEMETHODIMP CreateViewObject(HWND, REFIID, void**) noexcept override;
    IFACEMETHODIMP GetAttributesOf(UINT, PCUITEMID_CHILD_ARRAY, SFGAOF*) noexcept override;
    IFACEMETHODIMP GetUIObjectOf(HWND, UINT, PCUITEMID_CHILD_ARRAY, REFIID, UINT*, void**) noexcept override;
    IFACEMETHODIMP GetDisplayNameOf(PCUITEMID_CHILD, SHGDNF, STRRET*) noexcept override;
    IFACEMETHODIMP SetNameOf(HWND, PCUITEMID_CHILD, PCWSTR, SHGDNF, PITEMID_CHILD*) noexcept override;

    IFACEMETHODIMP GetDefaultSearchGUID(GUID*) noexcept override;
    IFACEMETHODIMP EnumSearches(IEnumExtraSearch**) noexcept override;
    IFACEMETHODIMP GetDefaultColumn(DWORD, ULONG*, ULONG*) noexcept override;
    IFACEMETHODIMP GetDefaultColumnState(UINT, SHCOLSTATEF*) noexcept override;
    IFACEMETHODIMP GetDetailsEx(PCUITEMID_CHILD, const SHCOLUMNID*, VARIANT*) noexcept override;
    IFACEMETHODIMP GetDetailsOf(PCUITEMID_CHILD, UINT, SHELLDETAILS*) noexcept override;
    IFACEMETHODIMP MapColumnToSCID(UINT, SHCOLUMNID*) noexcept override;

private:
    ~ShellFolder();
    std::atomic<ULONG> references_{1};
    PIDLIST_ABSOLUTE root_pidl_{nullptr};
};
}
