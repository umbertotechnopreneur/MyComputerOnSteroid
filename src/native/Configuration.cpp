// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; unified native configuration; 2026-10-17.
 * Human guidance: Umberto Giacobbi; one menu command, tabbed settings and setup guide.
 * Evidence: docs/VALIDATION.md; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#include "Configuration.hpp"
#include "Dialog.hpp"
#include "ThisPcEntries.hpp"
#include "ViewDialog.hpp"
#include "RecentDialog.hpp"
#include "../ProjectIdentity.hpp"
#include "../../resources/resource.h"
#include <CommCtrl.h>
#include <ExDisp.h>
#include <ShlGuid.h>
#include <Propkey.h>
#include <array>
#include <optional>

namespace mcos
{
namespace
{
constexpr UINT ReloadFavorites = WM_APP + 21;
struct FavoritesState
{
    std::vector<CatalogItem> entries;
    HIMAGELIST images{};
    ~FavoritesState() { if (images) ImageList_Destroy(images); }
};

std::optional<CatalogItem> selected_favorite(HWND dialog, const FavoritesState& state)
{
    const int selected = ListView_GetNextItem(GetDlgItem(dialog, IDC_FAVORITES_LIST), -1, LVNI_SELECTED);
    if (selected < 0 || static_cast<size_t>(selected) >= state.entries.size()) return {};
    return state.entries[static_cast<size_t>(selected)];
}

void load_favorites(HWND dialog, FavoritesState& state)
{
    state.entries.clear();
    for (auto& item : enumerate_catalog(false))
        if (is_owned_favorite(item)) state.entries.push_back(std::move(item));
    const HWND list = GetDlgItem(dialog, IDC_FAVORITES_LIST);
    ListView_DeleteAllItems(list);
    const int dpi = static_cast<int>(GetDpiForWindow(dialog));
    const int edge = MulDiv(96, dpi, 96);
    const HIMAGELIST images = ImageList_Create(edge, edge, ILC_COLOR32 | ILC_MASK, 1, 8);
    if (!images) check(E_OUTOFMEMORY);
    ListView_SetImageList(list, images, LVSIL_NORMAL);
    if (state.images) ImageList_Destroy(state.images);
    state.images = images;
    ListView_SetExtendedListViewStyle(list, LVS_EX_DOUBLEBUFFER | LVS_EX_BORDERSELECT);
    ListView_SetIconSpacing(list, MulDiv(144, dpi, 96), MulDiv(136, dpi, 96));
    for (size_t index = 0; index < state.entries.size(); ++index)
    {
        const auto& item = state.entries[index];
        const auto name = item_name(item);
        HICON icon{};
        // The Shell image factory reads the reference's icon; it does not run
        // the item. Image-list ownership is separate from the returned bitmap.
        ComPtr<IShellItemImageFactory> factory;
        const auto reference = parse_pidl(item.path);
        if (SUCCEEDED(SHCreateItemFromIDList(reference.get(), IID_PPV_ARGS(&factory))))
        {
            HBITMAP bitmap{};
            if (SUCCEEDED(factory->GetImage({edge, edge}, SIIGBF_ICONONLY, &bitmap)))
            {
                const int image = ImageList_Add(images, bitmap, nullptr);
                DeleteObject(bitmap);
                if (image >= 0)
                {
                    LVITEMW entry{};
                    entry.mask = LVIF_TEXT | LVIF_IMAGE;
                    entry.iItem = static_cast<int>(index);
                    entry.pszText = const_cast<wchar_t*>(name.c_str());
                    entry.iImage = image;
                    if (ListView_InsertItem(list, &entry) < 0) check(E_FAIL);
                    continue;
                }
            }
        }
        SHSTOCKICONINFO fallback{sizeof(fallback)};
        int image = -1;
        if (SUCCEEDED(SHGetStockIconInfo(SIID_DOCNOASSOC, SHGSI_ICON | SHGSI_LARGEICON, &fallback)))
        {
            icon = fallback.hIcon;
            image = ImageList_AddIcon(images, icon);
            DestroyIcon(icon);
        }
        LVITEMW entry{};
        entry.mask = LVIF_TEXT | LVIF_IMAGE;
        entry.iItem = static_cast<int>(index);
        entry.pszText = const_cast<wchar_t*>(name.c_str());
        entry.iImage = image;
        if (ListView_InsertItem(list, &entry) < 0) check(E_FAIL);
    }
    EnableWindow(GetDlgItem(dialog, IDC_FAVORITES_REMOVE), FALSE);
    EnableWindow(GetDlgItem(dialog, IDC_FAVORITES_ICON), FALSE);
}

INT_PTR CALLBACK favorites_page(HWND dialog, UINT message, WPARAM first, LPARAM second) noexcept
{
    if (message == WM_INITDIALOG) SetWindowLongPtrW(dialog, DWLP_USER, second);
    auto* state = reinterpret_cast<FavoritesState*>(GetWindowLongPtrW(dialog, DWLP_USER));
    if (!state) return FALSE;
    const HRESULT status = guarded([&] {
        if (message == WM_INITDIALOG || message == ReloadFavorites || message == WM_APP + 19)
        {
            load_favorites(dialog, *state);
            return S_OK;
        }
        if (message == WM_NOTIFY)
        {
            const auto* notice = reinterpret_cast<NMHDR*>(second);
            if (notice->idFrom != IDC_FAVORITES_LIST || notice->code != LVN_ITEMCHANGED) return S_FALSE;
            const bool selected = selected_favorite(dialog, *state).has_value();
            EnableWindow(GetDlgItem(dialog, IDC_FAVORITES_REMOVE), selected);
            EnableWindow(GetDlgItem(dialog, IDC_FAVORITES_ICON), selected);
            return S_OK;
        }
        if (message != WM_COMMAND) return S_FALSE;
        switch (LOWORD(first))
        {
        case IDC_FAVORITES_ADD_FOLDER: choose_favorite(dialog, Section::folders); break;
        case IDC_FAVORITES_ADD_APP: choose_favorite(dialog, Section::apps); break;
        case IDC_FAVORITES_ADD_FILE: choose_favorite(dialog, Section::files); break;
        case IDC_FAVORITES_REMOVE:
        {
            const auto item = selected_favorite(dialog, *state);
            if (item) remove_favorite(*item);
            break;
        }
        case IDC_FAVORITES_ICON:
        {
            const auto item = selected_favorite(dialog, *state);
            if (item) choose_favorite_icon(dialog, {*item});
            break;
        }
        default: return S_FALSE;
        }
        load_favorites(dialog, *state);
        return S_OK;
    });
    if (FAILED(status))
    {
        if (message == WM_INITDIALOG) fail_native_dialog(dialog, status);
        else report_error(dialog, status, IDS_ERROR_CONFIGURATION);
    }
    return status != S_FALSE;
}

std::unique_ptr<NativeDialogPage> create_favorites_page(HWND parent)
{
    auto state = std::make_shared<FavoritesState>();
    const DialogAnchor anchors[]{
        {IDC_DIALOG_HEADING, true}, {IDC_FAVORITES_DESCRIPTION, true}, {IDC_DIALOG_HEADER_LINE, true},
        {IDC_FAVORITES_LIST, true, false, true},
        {IDC_DIALOG_FOOTER_LINE, true, false, false, true},
        {IDC_FAVORITES_REMOVE, false, false, false, true}, {IDC_FAVORITES_ICON, false, false, false, true}};
    return NativeDialogPage::create(IDD_FAVORITES_PAGE, parent, favorites_page,
        reinterpret_cast<LPARAM>(state.get()), anchors, state);
}

// Refresh only this namespace, through public automation/services. No private
// Explorer classes, retained host references or process injection are used.
void refresh_launcher_views()
{
    const auto root = parse_pidl(identity::parsing_name);
    const auto settings = view_settings();
    ComPtr<IShellWindows> windows;
    check(CoCreateInstance(CLSID_ShellWindows, nullptr, CLSCTX_LOCAL_SERVER, IID_PPV_ARGS(&windows)));
    long count{};
    check(windows->get_Count(&count));
    for (long index = 0; index < count; ++index)
    {
        VARIANT value{};
        value.vt = VT_I4;
        value.lVal = index;
        ComPtr<IDispatch> dispatch;
        ComPtr<IServiceProvider> services;
        ComPtr<IShellBrowser> browser;
        ComPtr<IShellView> view;
        ComPtr<IFolderView2> folder_view;
        ComPtr<IPersistFolder2> folder;
        if (FAILED(windows->Item(value, &dispatch)) || !dispatch ||
            FAILED(dispatch.As(&services)) ||
            FAILED(services->QueryService(SID_STopLevelBrowser, IID_PPV_ARGS(&browser))) ||
            FAILED(browser->QueryActiveShellView(&view)) || FAILED(view.As(&folder_view)) ||
            FAILED(folder_view->GetFolder(IID_PPV_ARGS(&folder)))) continue;
        PIDLIST_ABSOLUTE actual{};
        if (FAILED(folder->GetCurFolder(&actual))) continue;
        const Pidl owned(actual);
        if (!ILIsEqual(root.get(), owned.get())) continue;
        PROPERTYKEY group = PKEY_ItemTypeText;
        if (settings.grouping == GroupMode::extension) group = PKEY_FileExtension;
        else if (settings.grouping == GroupMode::families) group = PKEY_KindText;
        else if (settings.grouping == GroupMode::none) group = PKEY_Null;
        check(folder_view->SetGroupBy(group, TRUE));
        const auto direction = settings.descending ? SORT_DESCENDING : SORT_ASCENDING;
        const std::array columns{
            SORTCOLUMN{settings.order == ItemOrder::date ? PKEY_ItemDate : PKEY_ItemNameDisplay, direction},
            SORTCOLUMN{PKEY_ItemNameDisplay, SORT_ASCENDING}};
        check(folder_view->SetSortColumns(columns.data(), settings.order == ItemOrder::date ? 2 : 1));
        const auto empty = settings.query.empty() ? L"" : text(IDS_NO_SEARCH_RESULTS);
        check(folder_view->SetText(FVST_EMPTYTEXT, empty.c_str()));
        check(view->Refresh());
    }
    SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, root.get(), nullptr);
}

struct ConfigurationState
{
    ViewSettings view{view_settings()};
    RecentSettings recents{recent_settings()};
    std::array<std::unique_ptr<NativeDialogPage>, 5> pages;
};

void layout_pages(HWND dialog, ConfigurationState& state)
{
    const HWND tabs = GetDlgItem(dialog, IDC_CONFIG_TABS);
    RECT area{};
    check_win32(GetClientRect(tabs, &area));
    TabCtrl_AdjustRect(tabs, FALSE, &area);
    MapWindowPoints(tabs, dialog, reinterpret_cast<POINT*>(&area), 2);
    const int selected = TabCtrl_GetCurSel(tabs);
    for (size_t index = 0; index < state.pages.size(); ++index)
    {
        if (!state.pages[index]) continue;
        const HWND page = state.pages[index]->window();
        check_win32(SetWindowPos(page, HWND_TOP, area.left, area.top,
            area.right - area.left, area.bottom - area.top, SWP_NOACTIVATE));
        ShowWindow(page, static_cast<int>(index) == selected ? SW_SHOW : SW_HIDE);
    }
    const bool preferences = selected == 2 || selected == 3;
    ShowWindow(GetDlgItem(dialog, IDOK), preferences ? SW_SHOW : SW_HIDE);
    EnableWindow(GetDlgItem(dialog, IDOK), preferences);
}

INT_PTR CALLBACK configuration_dialog(HWND dialog, UINT message, WPARAM first, LPARAM second) noexcept
{
    if (message == WM_INITDIALOG) SetWindowLongPtrW(dialog, DWLP_USER, second);
    auto* state = reinterpret_cast<ConfigurationState*>(GetWindowLongPtrW(dialog, DWLP_USER));
    if (!state) return FALSE;
    if ((message == WM_CLOSE || message == WM_COMMAND) && !IsWindowEnabled(dialog)) return TRUE;
    const auto status = guarded([&] {
        if (message == WM_INITDIALOG)
        {
            const HWND tabs = GetDlgItem(dialog, IDC_CONFIG_TABS);
            const std::array labels{IDS_TAB_THIS_PC, IDS_TAB_FAVORITES, IDS_TAB_VIEW, IDS_TAB_RECENTS, IDS_TAB_ABOUT};
            for (size_t index = 0; index < labels.size(); ++index)
            {
                auto label = text(labels[index]);
                TCITEMW item{};
                item.mask = TCIF_TEXT;
                item.pszText = label.data();
                if (TabCtrl_InsertItem(tabs, static_cast<int>(index), &item) < 0) check(E_FAIL);
            }
            state->pages[0] = create_this_pc_page(dialog);
            state->pages[1] = create_favorites_page(dialog);
            state->pages[2] = create_view_page(dialog, state->view);
            state->pages[3] = create_recent_page(dialog, state->recents);
            state->pages[4] = create_about_page(dialog);
            SetDlgItemTextW(dialog, IDCANCEL, text(IDS_CLOSE).c_str());
            SetDlgItemTextW(dialog, IDC_CONFIG_STATUS, L"");
            layout_pages(dialog, *state);
            return S_OK;
        }
        if (message == WM_DESTROY)
        {
            // Child HWNDs must be gone before their controller state/fonts.
            for (auto& page : state->pages) page.reset();
            return S_OK;
        }
        if (message == WM_SIZE || message == WM_APP + 19)
        {
            layout_pages(dialog, *state);
            return S_OK;
        }
        if (message == WM_NOTIFY)
        {
            const auto* notice = reinterpret_cast<NMHDR*>(second);
            if (notice->idFrom != IDC_CONFIG_TABS || notice->code != TCN_SELCHANGE) return S_FALSE;
            SetDlgItemTextW(dialog, IDC_CONFIG_STATUS, L"");
            layout_pages(dialog, *state);
            return S_OK;
        }
        if (message != WM_COMMAND) return S_FALSE;
        if (LOWORD(first) == IDCANCEL)
        {
            EndDialog(dialog, IDCANCEL);
            return S_OK;
        }
        if (LOWORD(first) == IDC_CONFIG_GUIDE)
            return launch_process(dialog, module_path(nullptr).parent_path() / L"Readme Setup.txt", L"");
        if (LOWORD(first) != IDOK) return S_FALSE;
        const int selected = TabCtrl_GetCurSel(GetDlgItem(dialog, IDC_CONFIG_TABS));
        if (selected != 2 && selected != 3) return S_OK;
        HRESULT accepted{E_UNEXPECTED};
        SendMessageW(state->pages[2]->window(), ReadDialogPage, 0, reinterpret_cast<LPARAM>(&accepted));
        if (accepted != S_OK)
        {
            TabCtrl_SetCurSel(GetDlgItem(dialog, IDC_CONFIG_TABS), 2);
            layout_pages(dialog, *state);
            return accepted;
        }
        SendMessageW(state->pages[3]->window(), ReadDialogPage, 0, reinterpret_cast<LPARAM>(&accepted));
        if (FAILED(accepted))
        {
            TabCtrl_SetCurSel(GetDlgItem(dialog, IDC_CONFIG_TABS), 3);
            layout_pages(dialog, *state);
            return accepted;
        }
        save_view_settings(state->view);
        save_recent_settings(state->recents);
        // Persistence and live view application are separate failure boundaries.
        refresh_launcher_views();
        SetDlgItemTextW(dialog, IDC_CONFIG_STATUS, text(IDS_CONFIG_SAVED).c_str());
        return S_OK;
    });
    if (FAILED(status))
    {
        report_error(dialog, status, IDS_ERROR_CONFIGURATION);
        if (message == WM_INITDIALOG) EndDialog(dialog, IDCANCEL);
    }
    return status != S_FALSE;
}
}

void show_configuration(const RECT& origin)
{
    ConfigurationState state;
    const DialogAnchor anchors[]{
        {IDC_CONFIG_TABS, true, false, true}, {IDC_DIALOG_FOOTER_LINE, true, false, false, true},
        {IDC_CONFIG_GUIDE, false, false, false, true}, {IDC_CONFIG_STATUS, true, false, false, true},
        {IDOK, false, true, false, true}, {IDCANCEL, false, true, false, true}};
    show_native_dialog(IDD_CONFIGURATION, nullptr, configuration_dialog,
        reinterpret_cast<LPARAM>(&state), anchors, &origin);
}
}
