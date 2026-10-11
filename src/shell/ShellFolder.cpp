// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; empty namespace provider and DefView construction; 2026-10-11.
 * Human guidance: Umberto Giacobbi; native C++, explicit validation evidence.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#include "ShellFolder.hpp"
#include "Module.hpp"
#include "../ProjectIdentity.hpp"
#include "../../resources/resource.h"

#include <Propkey.h>
#include <Shlwapi.h>
#include <CommCtrl.h>
#include <new>

namespace mcos::shell
{
namespace
{
// An explicit empty enumerator is the bootstrap state, not fabricated user data.
class EmptyEnumerator final : public IEnumIDList
{
public:
    EmptyEnumerator() noexcept { ++live_objects; }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** result) noexcept override
    {
        if (!result) return E_POINTER;
        *result = nullptr;
        if (iid != IID_IUnknown && iid != IID_IEnumIDList) return E_NOINTERFACE;
        *result = static_cast<IEnumIDList*>(this);
        AddRef();
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef() noexcept override { return ++references_; }

    ULONG STDMETHODCALLTYPE Release() noexcept override
    {
        const ULONG remaining = --references_;
        if (remaining == 0) delete this;
        return remaining;
    }

    HRESULT STDMETHODCALLTYPE Next(ULONG count, PITEMID_CHILD* items, ULONG* fetched) noexcept override
    {
        if (fetched) *fetched = 0;
        if (count == 0) return S_OK;
        if (!items || (!fetched && count != 1)) return E_POINTER;
        for (ULONG index = 0; index < count; ++index) items[index] = nullptr;
        return S_FALSE;
    }

    HRESULT STDMETHODCALLTYPE Skip(ULONG count) noexcept override { return count == 0 ? S_OK : S_FALSE; }
    HRESULT STDMETHODCALLTYPE Reset() noexcept override { return S_OK; }

    HRESULT STDMETHODCALLTYPE Clone(IEnumIDList** result) noexcept override
    {
        if (!result) return E_POINTER;
        *result = new (std::nothrow) EmptyEnumerator();
        return *result ? S_OK : E_OUTOFMEMORY;
    }

private:
    ~EmptyEnumerator() { --live_objects; }
    std::atomic<ULONG> references_{1};
};

// source: borrowed null-terminated string.
// result: receives a CoTaskMem-owned STRRET string, released by the Shell.
HRESULT make_string(PCWSTR source, STRRET* result) noexcept
{
    if (!result) return E_POINTER;
    *result = {};
    result->uType = STRRET_WSTR;
    return SHStrDupW(source, &result->pOleStr);
}

HRESULT make_name_column(STRRET* result) noexcept
{
    // Resolve this DLL without changing its reference count or using DllMain.
    HMODULE module = nullptr;
    const auto address = reinterpret_cast<LPCWSTR>(&identity::launcher_clsid);
    if (!GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, address, &module))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    wchar_t name[64]{};
    if (LoadStringW(module, IDS_COLUMN_NAME, name, ARRAYSIZE(name)) == 0)
    {
        return HRESULT_FROM_WIN32(ERROR_RESOURCE_NAME_NOT_FOUND);
    }
    return make_string(name, result);
}
}

ShellFolder::ShellFolder() noexcept { ++live_objects; }

ShellFolder::~ShellFolder()
{
    CoTaskMemFree(root_pidl_);
    --live_objects;
}

HRESULT ShellFolder::QueryInterface(REFIID iid, void** result) noexcept
{
    if (!result) return E_POINTER;
    *result = nullptr;
    if (iid == IID_IUnknown || iid == IID_IShellFolder || iid == IID_IShellFolder2)
        *result = static_cast<IShellFolder2*>(this);
    else if (iid == IID_IPersist || iid == IID_IPersistFolder || iid == IID_IPersistFolder2)
        *result = static_cast<IPersistFolder2*>(this);
    else
        return E_NOINTERFACE;
    AddRef();
    return S_OK;
}

ULONG ShellFolder::AddRef() noexcept { return ++references_; }

ULONG ShellFolder::Release() noexcept
{
    const ULONG remaining = --references_;
    if (remaining == 0) delete this;
    return remaining;
}

HRESULT ShellFolder::GetClassID(CLSID* class_id) noexcept
{
    if (!class_id) return E_POINTER;
    *class_id = identity::launcher_clsid;
    return S_OK;
}

// pidl: borrowed absolute PIDL; clone before replacing owned state.
HRESULT ShellFolder::Initialize(PCIDLIST_ABSOLUTE pidl) noexcept
{
    if (!pidl) return E_INVALIDARG;
    PIDLIST_ABSOLUTE clone = ILCloneFull(pidl);
    if (!clone) return E_OUTOFMEMORY;
    CoTaskMemFree(root_pidl_);
    root_pidl_ = clone;
    return S_OK;
}

// pidl: receives a new CoTaskMem-owned absolute PIDL.
HRESULT ShellFolder::GetCurFolder(PIDLIST_ABSOLUTE* pidl) noexcept
{
    if (!pidl) return E_POINTER;
    *pidl = nullptr;
    if (!root_pidl_) return E_UNEXPECTED;
    *pidl = ILCloneFull(root_pidl_);
    return *pidl ? S_OK : E_OUTOFMEMORY;
}

HRESULT ShellFolder::ParseDisplayName(
    HWND, IBindCtx*, PWSTR, ULONG* eaten, PIDLIST_RELATIVE* pidl, ULONG* attributes) noexcept
{
    if (eaten) *eaten = 0;
    if (attributes) *attributes = 0;
    if (!pidl) return E_POINTER;
    *pidl = nullptr;
    return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
}

HRESULT ShellFolder::EnumObjects(HWND, SHCONTF, IEnumIDList** result) noexcept
{
    if (!result) return E_POINTER;
    *result = new (std::nothrow) EmptyEnumerator();
    return *result ? S_OK : E_OUTOFMEMORY;
}

HRESULT ShellFolder::BindToObject(PCUIDLIST_RELATIVE, IBindCtx*, REFIID, void** result) noexcept
{
    if (!result) return E_POINTER;
    *result = nullptr;
    return E_NOINTERFACE;
}

HRESULT ShellFolder::BindToStorage(PCUIDLIST_RELATIVE, IBindCtx*, REFIID, void** result) noexcept
{
    if (!result) return E_POINTER;
    *result = nullptr;
    return E_NOINTERFACE;
}

HRESULT ShellFolder::CompareIDs(LPARAM, PCUIDLIST_RELATIVE, PCUIDLIST_RELATIVE) noexcept
{
    return E_NOTIMPL;
}

// iid: only IShellView is advertised by this bootstrap.
// result: receives the genuine system DefView object, not a replacement ListView.
HRESULT ShellFolder::CreateViewObject(HWND, REFIID iid, void** result) noexcept
{
    if (!result) return E_POINTER;
    *result = nullptr;
    if (iid != IID_IShellView) return E_NOINTERFACE;

    SFV_CREATE create{};
    create.cbSize = sizeof(create);
    create.pshf = this;
    IShellView* view = nullptr;
    const HRESULT status = SHCreateShellFolderView(&create, &view);
    if (SUCCEEDED(status)) *result = view;
    return status;
}

HRESULT ShellFolder::GetAttributesOf(UINT count, PCUITEMID_CHILD_ARRAY, SFGAOF* attributes) noexcept
{
    if (!attributes) return E_POINTER;
    if (count != 0)
    {
        *attributes = 0;
        return E_INVALIDARG;
    }
    *attributes &= SFGAO_FOLDER | SFGAO_BROWSABLE;
    return S_OK;
}

HRESULT ShellFolder::GetUIObjectOf(HWND, UINT, PCUITEMID_CHILD_ARRAY, REFIID, UINT*, void** result) noexcept
{
    if (!result) return E_POINTER;
    *result = nullptr;
    return E_NOINTERFACE;
}

HRESULT ShellFolder::GetDisplayNameOf(PCUITEMID_CHILD pidl, SHGDNF, STRRET* result) noexcept
{
    if (!result) return E_POINTER;
    *result = {};
    if (pidl && !ILIsEmpty(pidl)) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    return make_string(identity::product_name, result);
}

HRESULT ShellFolder::SetNameOf(HWND, PCUITEMID_CHILD, PCWSTR, SHGDNF, PITEMID_CHILD* renamed) noexcept
{
    if (renamed) *renamed = nullptr;
    return E_NOTIMPL;
}

HRESULT ShellFolder::GetDefaultSearchGUID(GUID* search_id) noexcept
{
    if (!search_id) return E_POINTER;
    *search_id = GUID_NULL;
    return E_NOTIMPL;
}

HRESULT ShellFolder::EnumSearches(IEnumExtraSearch** result) noexcept
{
    if (!result) return E_POINTER;
    *result = nullptr;
    return E_NOTIMPL;
}

HRESULT ShellFolder::GetDefaultColumn(DWORD, ULONG* sort_column, ULONG* display_column) noexcept
{
    if (!sort_column || !display_column) return E_POINTER;
    *sort_column = 0;
    *display_column = 0;
    return S_OK;
}

HRESULT ShellFolder::GetDefaultColumnState(UINT column, SHCOLSTATEF* state) noexcept
{
    if (!state) return E_POINTER;
    *state = 0;
    if (column != 0) return E_INVALIDARG;
    *state = SHCOLSTATE_TYPE_STR | SHCOLSTATE_ONBYDEFAULT;
    return S_OK;
}

HRESULT ShellFolder::GetDetailsEx(PCUITEMID_CHILD, const SHCOLUMNID*, VARIANT* value) noexcept
{
    if (!value) return E_POINTER;
    VariantInit(value);
    return E_NOTIMPL;
}

HRESULT ShellFolder::GetDetailsOf(PCUITEMID_CHILD pidl, UINT column, SHELLDETAILS* details) noexcept
{
    if (!details) return E_POINTER;
    *details = {};
    if (column != 0) return E_INVALIDARG;
    if (pidl) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    details->fmt = LVCFMT_LEFT;
    details->cxChar = 36;
    return make_name_column(&details->str);
}

HRESULT ShellFolder::MapColumnToSCID(UINT column, SHCOLUMNID* column_id) noexcept
{
    if (!column_id) return E_POINTER;
    *column_id = {};
    if (column != 0) return E_INVALIDARG;
    *column_id = PKEY_ItemNameDisplay;
    return S_OK;
}

HRESULT create_shell_folder(REFIID iid, void** result) noexcept
{
    if (!result) return E_POINTER;
    *result = nullptr;
    auto* folder = new (std::nothrow) ShellFolder();
    if (!folder) return E_OUTOFMEMORY;
    const HRESULT status = folder->QueryInterface(iid, result);
    folder->Release();
    return status;
}
}
