// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; native favorite menu adapter; 2026-10-11.
 * Human guidance: Umberto Giacobbi; native menus, remove references only.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#include "FavoriteMenu.hpp"
#include "Module.hpp"
#include "../core/Recents.hpp"
#include "../native/MenuIcons.hpp"
#include "../../resources/resource.h"
#include <array>
#include <algorithm>
#include <Strsafe.h>

namespace mcos::shell
{
namespace
{
class FavoriteMenu final : public IContextMenu3, public IObjectWithSite
{
public:
    FavoriteMenu(IContextMenu* native, std::vector<CatalogItem> items, HWND owner)
        : native_(native), items_(std::move(items)), owner_(owner) { ++live_objects; }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** result) noexcept override
    {
        if (!result) return E_POINTER;
        *result = nullptr;
        if (iid == IID_IUnknown || iid == IID_IContextMenu || iid == IID_IContextMenu2 || iid == IID_IContextMenu3)
            *result = static_cast<IContextMenu3*>(this);
        else if (iid == IID_IObjectWithSite) *result = static_cast<IObjectWithSite*>(this);
        else return E_NOINTERFACE;
        AddRef();
        return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() noexcept override { return ++references_; }
    ULONG STDMETHODCALLTYPE Release() noexcept override
    { const auto left = --references_; if (!left) delete this; return left; }

    HRESULT STDMETHODCALLTYPE QueryContextMenu(HMENU menu, UINT index, UINT first, UINT last, UINT flags) noexcept override
    {
        return guarded([&] {
            commands_.clear();
            const auto status = native_->QueryContextMenu(menu, index, first, last, flags);
            if (FAILED(status) || flags & CMF_DEFAULTONLY) return status;
            UINT used = HRESULT_CODE(status);
            const bool all_recent = std::all_of(items_.begin(), items_.end(), [](const auto& item) { return item.section == Section::recents; });
            const bool all_favorites = std::all_of(items_.begin(), items_.end(), [](const auto& item) { return is_favorite_section(item.section); });
            std::vector<UINT> labels;
            if (all_favorites) labels.push_back(IDS_CHANGE_ICON);
            if (all_recent) labels = {IDS_RECENT_ADD, IDS_RECENT_PIN, IDS_RECENT_UNPIN, IDS_RECENT_HIDE};
            if (!labels.empty() && first <= last && used < last - first + 1)
            {
                check_win32(InsertMenuW(menu, static_cast<UINT>(GetMenuItemCount(menu)), MF_BYPOSITION | MF_SEPARATOR, 0, nullptr));
                for (const auto label : labels)
                {
                    if (used >= last - first + 1) break;
                    check_win32(InsertMenuW(menu, static_cast<UINT>(GetMenuItemCount(menu)), MF_BYPOSITION | MF_STRING,
                        first + used, text(label).c_str()));
                    icons_.attach(menu, first + used, glyph(label), owner_);
                    commands_.push_back({used++, label});
                }
            }
            return MAKE_HRESULT(SEVERITY_SUCCESS, 0, used);
        });
    }

    HRESULT STDMETHODCALLTYPE GetCommandString(UINT_PTR command, UINT flags, UINT* reserved, LPSTR output, UINT size) noexcept override
    {
        return guarded([&] {
            for (const auto& entry : commands_)
            {
                if (entry.offset != command) continue;
                if (flags == GCS_VALIDATEA || flags == GCS_VALIDATEW) return S_OK;
                if (!output || !size) return E_POINTER;
                const auto value = flags == GCS_VERBA || flags == GCS_VERBW ? std::wstring(verb(entry.label)) : text(entry.label);
                if (flags & GCS_UNICODE) return StringCchCopyW(reinterpret_cast<PWSTR>(output), size, value.c_str());
                return WideCharToMultiByte(CP_ACP, 0, value.c_str(), -1, output, static_cast<int>(size), nullptr, nullptr)
                    ? S_OK : HRESULT_FROM_WIN32(GetLastError());
            }
            return native_->GetCommandString(command, flags, reserved, output, size);
        });
    }

    HRESULT STDMETHODCALLTYPE InvokeCommand(CMINVOKECOMMANDINFO* command) noexcept override
    {
        if (!command) return E_POINTER;
        if (command->cbSize < sizeof(CMINVOKECOMMANDINFO)) return E_INVALIDARG;
        UINT operation = IDS_ERROR_REMOVE;
        const auto status = guarded([&] {
            std::wstring action;
            const auto* extended = command->cbSize >= sizeof(CMINVOKECOMMANDINFOEX)
                ? reinterpret_cast<const CMINVOKECOMMANDINFOEX*>(command) : nullptr;
            if (extended && (command->fMask & CMIC_MASK_UNICODE) && extended->lpVerbW && !IS_INTRESOURCE(extended->lpVerbW))
                action = extended->lpVerbW;
            else if (IS_INTRESOURCE(command->lpVerb))
            {
                std::array<wchar_t, 128> name{};
                const auto result = GetCommandString(LOWORD(command->lpVerb), GCS_VERBW,
                    nullptr, reinterpret_cast<LPSTR>(name.data()), static_cast<UINT>(name.size()));
                if (SUCCEEDED(result)) action = name.data();
            }
            else if (command->lpVerb)
                for (const char* character = command->lpVerb; *character; ++character) action.push_back(static_cast<unsigned char>(*character));

            if (action == L"mcos.change-icon")
            { operation = IDS_ERROR_ICON; choose_favorite_icon(command->hwnd, items_); return S_OK; }
            if (action == L"mcos.recent-add")
            { operation = IDS_ERROR_RECENTS; add_recents_to_favorites(items_); return S_OK; }
            if (action == L"mcos.recent-pin" || action == L"mcos.recent-unpin")
            { operation = IDS_ERROR_RECENTS; pin_recents(items_, action == L"mcos.recent-pin"); return S_OK; }
            if (action == L"mcos.recent-hide")
            { operation = IDS_ERROR_RECENTS; hide_recents(items_); return S_OK; }

            if (_wcsicmp(action.c_str(), L"rename") == 0 &&
                std::any_of(items_.begin(), items_.end(), [](const auto& item) { return item.section == Section::recents; })) return E_ACCESSDENIED;

            if (_wcsicmp(action.c_str(), L"delete") != 0) return native_->InvokeCommand(command);
            // Do not resolve the .lnk or delegate deletion to a target. Core validates
            // ownership again at the mutation, including forged PIDLs and reparse points.
            std::vector<CatalogItem> recent;
            for (const auto& item : items_)
                if (item.section == Section::recents) recent.push_back(item);
                else remove_favorite(item);
            if (!recent.empty()) hide_recents(recent);
            return S_OK;
        });
        if (FAILED(status)) report_error(command->hwnd, status, operation);
        return status;
    }

    HRESULT STDMETHODCALLTYPE HandleMenuMsg(UINT message, WPARAM first, LPARAM second) noexcept override
    {
        ComPtr<IContextMenu2> menu;
        const auto status = native_.As(&menu);
        return FAILED(status) ? status : menu->HandleMenuMsg(message, first, second);
    }
    HRESULT STDMETHODCALLTYPE HandleMenuMsg2(UINT message, WPARAM first, LPARAM second, LRESULT* result) noexcept override
    {
        if (!result) return E_POINTER;
        *result = 0;
        ComPtr<IContextMenu3> menu;
        const auto status = native_.As(&menu);
        return FAILED(status) ? status : menu->HandleMenuMsg2(message, first, second, result);
    }
    HRESULT STDMETHODCALLTYPE SetSite(IUnknown* site) noexcept override
    {
        ComPtr<IObjectWithSite> object;
        const auto status = native_.As(&object);
        return status == E_NOINTERFACE ? S_OK : FAILED(status) ? status : object->SetSite(site);
    }
    HRESULT STDMETHODCALLTYPE GetSite(REFIID iid, void** result) noexcept override
    {
        if (!result) return E_POINTER;
        *result = nullptr;
        ComPtr<IObjectWithSite> object;
        const auto status = native_.As(&object);
        return FAILED(status) ? status : object->GetSite(iid, result);
    }

private:
    static MenuGlyph glyph(UINT label) noexcept
    {
        switch (label)
        {
        case IDS_RECENT_ADD: return MenuGlyph::favorite;
        case IDS_RECENT_PIN: return MenuGlyph::pin;
        case IDS_RECENT_UNPIN: return MenuGlyph::unpin;
        case IDS_RECENT_HIDE: return MenuGlyph::hide;
        default: return MenuGlyph::picture;
        }
    }
    static PCWSTR verb(UINT label) noexcept
    {
        switch (label)
        {
        case IDS_CHANGE_ICON: return L"mcos.change-icon";
        case IDS_RECENT_ADD: return L"mcos.recent-add";
        case IDS_RECENT_PIN: return L"mcos.recent-pin";
        case IDS_RECENT_UNPIN: return L"mcos.recent-unpin";
        case IDS_RECENT_HIDE: return L"mcos.recent-hide";
        default: return L"";
        }
    }
    struct Command { UINT offset; UINT label; };
    ~FavoriteMenu() { --live_objects; }
    std::atomic<ULONG> references_{1};
    ComPtr<IContextMenu> native_;
    std::vector<CatalogItem> items_;
    std::vector<Command> commands_;
    HWND owner_;
    MenuIcons icons_;
};
}
HRESULT favorite_menu(IContextMenu* native, std::vector<CatalogItem> items, HWND owner, REFIID iid, void** result) noexcept
{
    if (!result) return E_POINTER;
    *result = nullptr;
    if (!native || items.empty()) return E_INVALIDARG;
    return guarded([&] {
        auto* menu = new FavoriteMenu(native, std::move(items), owner);
        const auto status = menu->QueryInterface(iid, result);
        menu->Release();
        return status;
    });
}
}
