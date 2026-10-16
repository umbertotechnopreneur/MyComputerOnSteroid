// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; native searchable collection options; 2026-10-11.
 * Modified with AI: OpenAI Codex; separate dialog setup and validation; 2026-10-17.
 * Human guidance: Umberto Giacobbi; editable families and native Shell order.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */
#include "ViewDialog.hpp"
#include "../ProjectIdentity.hpp"
#include "../../resources/resource.h"
#include <array>

namespace mcos
{
namespace
{
struct DialogState
{
    ViewSettings* settings;
    std::array<HICON, 3> icons{};

    ~DialogState()
    {
        for (const auto icon : icons)
            if (icon) DestroyIcon(icon);
    }
};

void populate_choices(HWND dialog, UINT control, std::initializer_list<UINT> labels, unsigned selected)
{
    for (const auto resource : labels)
    {
        const auto label = text(resource);
        const auto result = SendDlgItemMessageW(dialog, control, CB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(label.c_str()));
        if (result < 0) check(E_FAIL);
    }

    SendDlgItemMessageW(dialog, control, CB_SETCURSEL, selected, 0);
}

unsigned selected_choice(HWND dialog, UINT control)
{
    const auto value = SendDlgItemMessageW(dialog, control, CB_GETCURSEL, 0, 0);
    if (value < 0) check(E_INVALIDARG);
    return static_cast<unsigned>(value);
}

// Stock icons belong to DialogState. Failure to decorate leaves usable controls.
void initialize_view_dialog(HWND dialog, DialogState& state)
{
    const auto& settings = *state.settings;
    const UINT controls[]{IDC_VIEW_SEARCH_ICON, IDC_VIEW_GROUP_ICON, IDC_VIEW_FAMILY_ICON};
    const SHSTOCKICONID stocks[]{SIID_FIND, SIID_FOLDER, SIID_DOCASSOC};

    for (size_t index = 0; index < state.icons.size(); ++index)
    {
        SHSTOCKICONINFO icon{sizeof(icon)};
        const auto status = SHGetStockIconInfo(stocks[index], SHGSI_ICON | SHGSI_SMALLICON, &icon);
        if (FAILED(status)) continue;

        state.icons[index] = icon.hIcon;
        SendDlgItemMessageW(dialog, controls[index], STM_SETICON,
            reinterpret_cast<WPARAM>(icon.hIcon), 0);
    }

    SendDlgItemMessageW(dialog, IDC_VIEW_QUERY, EM_SETLIMITTEXT, 256, 0);
    SendDlgItemMessageW(dialog, IDC_VIEW_FAMILIES, EM_SETLIMITTEXT, 16384, 0);
    SetDlgItemTextW(dialog, IDC_VIEW_QUERY, settings.query.c_str());
    SetDlgItemTextW(dialog, IDC_VIEW_FAMILIES, settings.families.c_str());

    populate_choices(dialog, IDC_VIEW_FIELD,
        {IDS_SEARCH_ANY, IDS_SEARCH_NAME, IDS_SEARCH_PATH, IDS_SEARCH_EXTENSION},
        static_cast<unsigned>(settings.field));
    populate_choices(dialog, IDC_VIEW_GROUP,
        {IDS_GROUP_SECTIONS, IDS_GROUP_EXTENSIONS, IDS_GROUP_FAMILIES, IDS_GROUP_NONE},
        static_cast<unsigned>(settings.grouping));
    populate_choices(dialog, IDC_VIEW_SORT, {IDS_SORT_NAME, IDS_SORT_DATE},
        static_cast<unsigned>(settings.order));
    populate_choices(dialog, IDC_VIEW_DIRECTION, {IDS_SORT_ASCENDING, IDS_SORT_DESCENDING},
        settings.descending ? 1u : 0u);
}

// Validate a local copy first. A warning or Cancel leaves the caller's settings
// intact; only a complete, accepted edit is handed back to the Shell command.
HRESULT accept_view_dialog(HWND dialog, DialogState& state)
{
    auto pending = *state.settings;
    pending.query = dialog_text(dialog, IDC_VIEW_QUERY);
    pending.families = dialog_text(dialog, IDC_VIEW_FAMILIES);

    const auto family_status = guarded([&] {
        parse_families(pending.families);
        return S_OK;
    });

    if (FAILED(family_status))
    {
        MessageBoxW(dialog, text(IDS_INVALID_FAMILIES).c_str(), identity::launcher_name, MB_OK | MB_ICONWARNING);
        SetFocus(GetDlgItem(dialog, IDC_VIEW_FAMILIES));
        return S_FALSE;
    }

    pending.field = static_cast<SearchField>(selected_choice(dialog, IDC_VIEW_FIELD));
    pending.grouping = static_cast<GroupMode>(selected_choice(dialog, IDC_VIEW_GROUP));
    pending.order = static_cast<ItemOrder>(selected_choice(dialog, IDC_VIEW_SORT));
    pending.descending = selected_choice(dialog, IDC_VIEW_DIRECTION) != 0;
    *state.settings = std::move(pending);
    return S_OK;
}

INT_PTR CALLBACK view_dialog(HWND dialog, UINT message, WPARAM first, LPARAM second) noexcept
{
    if (message == WM_INITDIALOG)
    {
        const auto status = guarded([&] {
            auto* state = reinterpret_cast<DialogState*>(second);
            SetWindowLongPtrW(dialog, DWLP_USER, second);
            initialize_view_dialog(dialog, *state);
            return S_OK;
        });

        if (FAILED(status))
        {
            report_error(dialog, status, IDS_ERROR_VIEW);
            EndDialog(dialog, IDCANCEL);
        }
        return TRUE;
    }

    if (message == WM_CTLCOLORSTATIC)
    {
        const auto control = GetDlgCtrlID(reinterpret_cast<HWND>(second));
        const bool is_hint = control == IDC_VIEW_SEARCH_HINT || control == IDC_VIEW_DATE_HINT ||
            control == IDC_VIEW_FAMILY_HINT;
        if (is_hint)
        {
            SetTextColor(reinterpret_cast<HDC>(first), GetSysColor(COLOR_GRAYTEXT));
            SetBkColor(reinterpret_cast<HDC>(first), GetSysColor(COLOR_3DFACE));
            return reinterpret_cast<INT_PTR>(GetSysColorBrush(COLOR_3DFACE));
        }
    }

    if (message != WM_COMMAND) return FALSE;

    const auto command = LOWORD(first);
    if (command == IDCANCEL)
    {
        EndDialog(dialog, IDCANCEL);
        return TRUE;
    }
    if (command == IDC_VIEW_CLEAR)
    {
        SetDlgItemTextW(dialog, IDC_VIEW_QUERY, L"");
        SetFocus(GetDlgItem(dialog, IDC_VIEW_QUERY));
        return TRUE;
    }
    if (command != IDOK) return FALSE;

    const auto status = guarded([&] {
        auto* state = reinterpret_cast<DialogState*>(GetWindowLongPtrW(dialog, DWLP_USER));
        if (!state || !state->settings) return E_UNEXPECTED;
        return accept_view_dialog(dialog, *state);
    });

    if (FAILED(status)) report_error(dialog, status, IDS_ERROR_VIEW);
    else if (status == S_OK) EndDialog(dialog, IDOK);
    return TRUE;
}
}

bool choose_view_settings(HWND owner, ViewSettings& settings)
{
    DialogState state{&settings};
    const auto result = DialogBoxParamW(current_module(), MAKEINTRESOURCEW(IDD_VIEW_OPTIONS),
        owner, view_dialog, reinterpret_cast<LPARAM>(&state));
    if (result == -1) check(HRESULT_FROM_WIN32(GetLastError()));
    return result == IDOK;
}
}
