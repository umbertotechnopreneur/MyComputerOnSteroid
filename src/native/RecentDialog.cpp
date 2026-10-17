// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; Win32 controls for recent filters; 2026-10-11.
 * Modified with AI: OpenAI Codex; shared dialog helpers and readable control flow; 2026-10-17.
 * Human guidance: Umberto Giacobbi; native UI, categories and name filtering.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#include "RecentDialog.hpp"
#include "../core/Recents.hpp"
#include "../../resources/resource.h"

namespace mcos
{
namespace
{
INT_PTR CALLBACK filter_dialog(HWND dialog, UINT message, WPARAM first, LPARAM second) noexcept
{
    if (message == WM_INITDIALOG)
    {
        const auto status = guarded([&] {
            auto* settings = reinterpret_cast<RecentSettings*>(second);
            SetWindowLongPtrW(dialog, DWLP_USER, second);
            CheckDlgButton(dialog, IDC_RECENT_ENABLED, settings->enabled ? BST_CHECKED : BST_UNCHECKED);
            SetDlgItemTextW(dialog, IDC_RECENT_NAME, settings->name.c_str());
            SetDlgItemTextW(dialog, IDC_RECENT_EXTENSION, settings->extension.c_str());
            SendDlgItemMessageW(dialog, IDC_RECENT_NAME, EM_SETLIMITTEXT, 256, 0);
            SendDlgItemMessageW(dialog, IDC_RECENT_EXTENSION, EM_SETLIMITTEXT, 32, 0);
            for (UINT resource : {IDS_TYPE_ALL, IDS_TYPE_DOCUMENTS, IDS_TYPE_IMAGES,
                IDS_TYPE_AUDIO, IDS_TYPE_VIDEO, IDS_TYPE_DEVELOPMENT, IDS_TYPE_OTHER})
            {
                const auto label = text(resource);
                const auto added = SendDlgItemMessageW(dialog, IDC_RECENT_CATEGORY,
                    CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str()));
                if (added < 0) check(E_FAIL);
            }
            SendDlgItemMessageW(dialog, IDC_RECENT_CATEGORY, CB_SETCURSEL, static_cast<WPARAM>(settings->category), 0);
            return S_OK;
        });
        if (FAILED(status))
        {
            report_error(dialog, status, IDS_ERROR_RECENTS);
            EndDialog(dialog, IDCANCEL);
        }
        return TRUE;
    }
    if (message == WM_COMMAND)
    {
        if (LOWORD(first) == IDCANCEL)
        {
            EndDialog(dialog, IDCANCEL);
            return TRUE;
        }
        if (LOWORD(first) == IDOK)
        {
            const auto status = guarded([&] {
                auto* settings = reinterpret_cast<RecentSettings*>(GetWindowLongPtrW(dialog, DWLP_USER));
                if (!settings) return E_UNEXPECTED;

                // Read all controls before assigning or saving the edited state.
                auto pending = *settings;
                pending.enabled = IsDlgButtonChecked(dialog, IDC_RECENT_ENABLED) == BST_CHECKED;
                pending.name = dialog_text(dialog, IDC_RECENT_NAME);
                pending.extension = dialog_text(dialog, IDC_RECENT_EXTENSION);
                const auto category = SendDlgItemMessageW(dialog, IDC_RECENT_CATEGORY, CB_GETCURSEL, 0, 0);
                if (category < 0) return E_INVALIDARG;

                pending.category = static_cast<RecentCategory>(category);
                save_recent_settings(pending);
                *settings = std::move(pending);
                return S_OK;
            });
            if (FAILED(status)) report_error(dialog, status, IDS_ERROR_RECENTS);
            else EndDialog(dialog, IDOK);
            return TRUE;
        }
    }
    return FALSE;
}
}
void choose_recent_filters(HWND owner)
{
    auto settings = recent_settings();
    const auto result = DialogBoxParamW(current_module(), MAKEINTRESOURCEW(IDD_RECENT_FILTERS),
        owner, filter_dialog, reinterpret_cast<LPARAM>(&settings));
    if (result == -1) check(HRESULT_FROM_WIN32(GetLastError()));
}
}
