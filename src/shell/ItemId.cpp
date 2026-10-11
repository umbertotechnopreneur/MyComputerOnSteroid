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

#include "ItemId.hpp"
#include "../core/Recents.hpp"
#include <cstring>
#include <limits>

namespace mcos::shell
{
Pidl make_item_id(const CatalogItem& item)
{
    const size_t bytes = 8 + (item.path.size() + 1) * sizeof(wchar_t);
    if (bytes > std::numeric_limits<USHORT>::max()) check(E_INVALIDARG);
    auto* data = static_cast<BYTE*>(CoTaskMemAlloc(bytes + sizeof(USHORT)));
    if (!data) check(E_OUTOFMEMORY);
    Pidl result(reinterpret_cast<ITEMIDLIST*>(data));
    const USHORT header[]{static_cast<USHORT>(bytes), 0x4D43, 1, static_cast<USHORT>(item.section)};
    memcpy(data, header, sizeof(header));
    memcpy(data + 8, item.path.c_str(), bytes - 8);
    memset(data + bytes, 0, sizeof(USHORT));
    return result;
}

CatalogItem read_item_id(PCUIDLIST_RELATIVE pidl)
{
    if (!pidl) check(E_INVALIDARG);
    USHORT bytes = 0; memcpy(&bytes, pidl, sizeof(bytes));
    if (bytes < 10 || bytes % 2) check(E_INVALIDARG);
    USHORT header[4]{}; memcpy(header, pidl, sizeof(header));
    if (header[1] != 0x4D43 || header[2] != 1 || header[3] < 1 || header[3] > 5) check(E_INVALIDARG);
    std::wstring path((bytes - 8) / sizeof(wchar_t), L'\0');
    memcpy(path.data(), reinterpret_cast<const BYTE*>(pidl) + 8, bytes - 8);
    if (path.empty() || path.back() != L'\0' || path.find(L'\0') != path.size() - 1) check(E_INVALIDARG);
    path.pop_back();
    if (path.empty()) check(E_INVALIDARG);
    CatalogItem item{static_cast<Section>(header[3]), std::move(path)};
    if (item.section == Section::drives)
    {
        if (item.path.size() != 3 || !iswalpha(item.path[0]) || item.path[1] != L':' || item.path[2] != L'\\')
            check(E_INVALIDARG);
    }
    else if (item.section == Section::recents ? !is_owned_recent(item) : !is_owned_favorite(item)) check(E_ACCESSDENIED);
    return item;
}

HRESULT native_ui_object(HWND owner, UINT count, PCUITEMID_CHILD_ARRAY items, REFIID iid, void** result)
{
    if (!result) return E_POINTER;
    *result = nullptr;
    if (!count || !items) return E_INVALIDARG;
    std::vector<Pidl> ids;
    std::vector<PCUITEMID_CHILD> children;
    ComPtr<IShellFolder> parent;
    Pidl parent_id;
    for (UINT index = 0; index < count; ++index)
    {
        ids.push_back(parse_pidl(read_item_id(items[index]).path));
        ComPtr<IShellFolder> next_parent;
        PCUITEMID_CHILD child = nullptr;
        check(SHBindToParent(ids.back().get(), IID_PPV_ARGS(&next_parent), &child));
        Pidl next_id(ILCloneFull(ids.back().get()));
        if (!next_id) check(E_OUTOFMEMORY);
        ILRemoveLastID(next_id.get());
        if (parent_id && !ILIsEqual(parent_id.get(), next_id.get()))
            return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
        if (!parent) { parent = next_parent; parent_id = std::move(next_id); }
        children.push_back(child);
    }
    return parent->GetUIObjectOf(owner, count, children.data(), iid, nullptr, result);
}
}
