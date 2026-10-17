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
#include "Dialog.hpp"
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

void initialize_view_dialog(HWND dialog, DialogState& state)
{
    const auto& settings = *state.settings;

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
        show_native_message(dialog, text(IDS_INVALID_FAMILIES));
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
            fail_native_dialog(dialog, status);
        }
        return TRUE;
    }

    if (message == ReadDialogPage)
    {
        auto* result = reinterpret_cast<HRESULT*>(second);
        if (!result) return FALSE;
        *result = guarded([&] {
            auto* state = reinterpret_cast<DialogState*>(GetWindowLongPtrW(dialog, DWLP_USER));
            return state ? accept_view_dialog(dialog, *state) : E_UNEXPECTED;
        });
        return TRUE;
    }
    if (message != WM_COMMAND) return FALSE;

    if (LOWORD(first) != IDC_VIEW_CLEAR) return FALSE;
    SetDlgItemTextW(dialog, IDC_VIEW_QUERY, L"");
    SetFocus(GetDlgItem(dialog, IDC_VIEW_QUERY));
    return TRUE;
}
}

std::unique_ptr<NativeDialogPage> create_view_page(HWND parent, ViewSettings& settings)
{
    auto state = std::make_shared<DialogState>();
    state->settings = &settings;
    const DialogAnchor anchors[]{
        {IDC_DIALOG_HEADING, true}, {IDC_VIEW_DESCRIPTION, true}, {IDC_DIALOG_HEADER_LINE, true},
        {IDC_VIEW_QUERY, true}, {IDC_VIEW_FIELD, true}, {IDC_VIEW_GROUP, true},
        {IDC_VIEW_DIRECTION_LABEL, false, true}, {IDC_VIEW_DIRECTION, false, true},
        {IDC_VIEW_FAMILIES, true, false, true},
        {IDC_DIALOG_FOOTER_LINE, true, false, false, true},
        {IDC_VIEW_CLEAR, false, false, false, true}};
    return NativeDialogPage::create(IDD_VIEW_OPTIONS, parent, view_dialog,
        reinterpret_cast<LPARAM>(state.get()), anchors, state);
}
}
