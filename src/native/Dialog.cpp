// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; shared native dialog presentation; 2026-10-17.
 * Human guidance: Umberto Giacobbi; DPI, headers, source monitor and About.
 * Evidence: docs/VALIDATION.md; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#include "Dialog.hpp"
#include "../ProjectIdentity.hpp"
#include "../../resources/resource.h"
#include <CommCtrl.h>
#include <algorithm>
#include <vector>
#include <cstring>

namespace mcos
{
namespace
{
constexpr wchar_t FrameProperty[] = L"MyComputerOnSteroid.DialogFrame";
constexpr UINT RefreshDpi = WM_APP + 19;

struct ControlLayout
{
    HWND window;
    RECT bounds;
    DialogAnchor anchor;
};

struct DialogFrame
{
    UINT resource;
    DLGPROC callback;
    LPARAM parameter;
    RECT origin;
    std::vector<DialogAnchor> anchors;
    std::vector<ControlLayout> controls;
    SIZE client{};
    SIZE minimum{};
    HFONT heading{};
    HICON small_icon{};
    HICON large_icon{};
    HICON about_icon{};
    HRESULT failure{S_OK};
    bool ready{};
    bool embedded{};

    ~DialogFrame()
    {
        if (heading) DeleteObject(heading);
        if (small_icon) DestroyIcon(small_icon);
        if (large_icon) DestroyIcon(large_icon);
        if (about_icon) DestroyIcon(about_icon);
    }
};

bool usable_owner(HWND owner) noexcept
{
    return owner && owner != GetDesktopWindow() && owner != GetShellWindow() &&
        IsWindow(owner) && !IsIconic(owner);
}

// This hidden, owned positioning window supplies the initial monitor/DPI when
// an elevated process has a source rectangle but cannot retain a modal owner.
struct PositionOwner
{
    HWND window{};
    ~PositionOwner() { if (window) DestroyWindow(window); }
};

void place_dialog(HWND dialog, const RECT& origin)
{
    const HMONITOR monitor = MonitorFromRect(&origin, MONITOR_DEFAULTTONEAREST);
    MONITORINFO info{sizeof(info)};
    check_win32(GetMonitorInfoW(monitor, &info));

    // Move to the source monitor first, letting PMv2 select its actual DPI.
    check_win32(SetWindowPos(dialog, nullptr, origin.left, origin.top, 0, 0,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE));
    RECT bounds{};
    check_win32(GetWindowRect(dialog, &bounds));
    const LONG width = bounds.right - bounds.left;
    const LONG height = bounds.bottom - bounds.top;
    const LONG left = origin.left + (origin.right - origin.left - width) / 2;
    const LONG top = origin.top + (origin.bottom - origin.top - height) / 2;
    const LONG max_left = std::max(info.rcWork.left, info.rcWork.right - width);
    const LONG max_top = std::max(info.rcWork.top, info.rcWork.bottom - height);
    check_win32(SetWindowPos(dialog, nullptr,
        std::clamp(left, info.rcWork.left, max_left),
        std::clamp(top, info.rcWork.top, max_top), 0, 0,
        SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE));
}

BOOL CALLBACK translate_control(HWND control, LPARAM parameter) noexcept
{
    const auto status = guarded([&] {
        wchar_t name[32]{};
        GetClassNameW(control, name, ARRAYSIZE(name));
        const int id = GetDlgCtrlID(control);
        if (id > 2 && (wcscmp(name, L"Static") == 0 || wcscmp(name, L"Button") == 0 ||
            wcscmp(name, WC_LINK) == 0))
        {
            // Every labelled control has a matching localized string resource.
            if (id != IDC_THIS_PC_PREVIEW && id != IDC_THIS_PC_MODE && id != IDC_ABOUT_ICON && id != IDC_CONFIG_STATUS &&
                id != IDC_DIALOG_HEADING && id != IDC_DIALOG_HEADER_LINE && id != IDC_DIALOG_FOOTER_LINE)
                check_win32(SetWindowTextW(control, text(static_cast<UINT>(id)).c_str()));
        }
        return S_OK;
    });
    if (FAILED(status)) *reinterpret_cast<HRESULT*>(parameter) = status;
    return SUCCEEDED(status);
}

void update_heading(HWND dialog, DialogFrame& frame)
{
    const UINT dpi = GetDpiForWindow(dialog);
    const auto module = current_module();
    const auto load_icon = [&](int metric) {
        const int edge = GetSystemMetricsForDpi(metric, dpi);
        return reinterpret_cast<HICON>(LoadImageW(module, MAKEINTRESOURCEW(IDI_APP),
            IMAGE_ICON, edge, edge, 0));
    };
    const HICON small_handle = load_icon(SM_CXSMICON);
    const HICON large_handle = load_icon(SM_CXICON);
    if (small_handle)
    {
        SendMessageW(dialog, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(small_handle));
        if (frame.small_icon) DestroyIcon(frame.small_icon);
        frame.small_icon = small_handle;
    }
    if (large_handle)
    {
        SendMessageW(dialog, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(large_handle));
        if (frame.large_icon) DestroyIcon(frame.large_icon);
        frame.large_icon = large_handle;
    }
    if (GetDlgItem(dialog, IDC_ABOUT_ICON))
    {
        const int edge = MulDiv(64, static_cast<int>(dpi), 96);
        const auto icon = reinterpret_cast<HICON>(LoadImageW(module, MAKEINTRESOURCEW(IDI_APP),
            IMAGE_ICON, edge, edge, 0));
        if (icon)
        {
            SendDlgItemMessageW(dialog, IDC_ABOUT_ICON, STM_SETICON, reinterpret_cast<WPARAM>(icon), 0);
            if (frame.about_icon) DestroyIcon(frame.about_icon);
            frame.about_icon = icon;
        }
    }
    LOGFONTW font{};
    const auto base = reinterpret_cast<HFONT>(SendMessageW(dialog, WM_GETFONT, 0, 0));
    if (!GetObjectW(base, sizeof(font), &font)) return;
    font.lfWeight = FW_SEMIBOLD;
    font.lfHeight = MulDiv(font.lfHeight, 11, 9);
    const HFONT heading = CreateFontIndirectW(&font);
    if (!heading) return;
    SendDlgItemMessageW(dialog, IDC_DIALOG_HEADING, WM_SETFONT,
        reinterpret_cast<WPARAM>(heading), TRUE);
    if (frame.heading) DeleteObject(frame.heading);
    frame.heading = heading;
}

struct HeaderShift
{
    HWND dialog;
    int boundary;
    int offset;
    HRESULT status{S_OK};
};

BOOL CALLBACK shift_header_content(HWND control, LPARAM parameter) noexcept
{
    auto& shift = *reinterpret_cast<HeaderShift*>(parameter);
    if (GetParent(control) != shift.dialog) return TRUE;
    RECT bounds{};
    if (!GetWindowRect(control, &bounds))
    {
        shift.status = HRESULT_FROM_WIN32(GetLastError());
        return FALSE;
    }
    MapWindowPoints(nullptr, shift.dialog, reinterpret_cast<POINT*>(&bounds), 2);
    if (bounds.top >= shift.boundary && !SetWindowPos(control, nullptr,
        bounds.left, bounds.top + shift.offset, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE))
    {
        shift.status = HRESULT_FROM_WIN32(GetLastError());
        return FALSE;
    }
    return TRUE;
}

// Measure localized copy with the real dialog font. Only the header's text and
// whitespace change height; the standard edit/button metrics stay untouched.
void fit_header(HWND dialog, UINT resource)
{
    UINT description{};
    switch (resource)
    {
    case IDD_THIS_PC_ENTRIES: description = IDC_THIS_PC_DESCRIPTION; break;
    case IDD_VIEW_OPTIONS: description = IDC_VIEW_DESCRIPTION; break;
    case IDD_RECENT_FILTERS: description = IDC_RECENT_DESCRIPTION; break;
    case IDD_ABOUT: description = IDC_ABOUT_DESCRIPTION; break;
    case IDD_FAVORITES_PAGE: description = IDC_FAVORITES_DESCRIPTION; break;
    default: return;
    }
    const HWND control = GetDlgItem(dialog, description);
    const auto copy = dialog_text(dialog, description);
    RECT bounds{};
    check_win32(GetWindowRect(control, &bounds));
    MapWindowPoints(nullptr, dialog, reinterpret_cast<POINT*>(&bounds), 2);
    RECT measured{0, 0, bounds.right - bounds.left, 0};
    const HDC context = GetDC(dialog);
    check_win32(context != nullptr);
    const auto font = reinterpret_cast<HFONT>(SendMessageW(control, WM_GETFONT, 0, 0));
    const HGDIOBJ previous = font ? SelectObject(context, font) : nullptr;
    const int height = DrawTextW(context, copy.c_str(), static_cast<int>(copy.size()), &measured,
        DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX);
    if (previous) SelectObject(context, previous);
    ReleaseDC(dialog, context);
    if (!height) return;
    check_win32(SetWindowPos(control, nullptr, 0, 0, bounds.right - bounds.left, height,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE));
    RECT spacing{0, 0, 0, 6};
    check_win32(MapDialogRect(dialog, &spacing));
    int header_bottom = bounds.top + height + spacing.bottom;
    if (const HWND icon = GetDlgItem(dialog, IDC_ABOUT_ICON))
    {
        RECT icon_bounds{};
        check_win32(GetWindowRect(icon, &icon_bounds));
        MapWindowPoints(nullptr, dialog, reinterpret_cast<POINT*>(&icon_bounds), 2);
        header_bottom = std::max(header_bottom, static_cast<int>(icon_bounds.bottom + spacing.bottom));
    }
    RECT line{};
    check_win32(GetWindowRect(GetDlgItem(dialog, IDC_DIALOG_HEADER_LINE), &line));
    MapWindowPoints(nullptr, dialog, reinterpret_cast<POINT*>(&line), 2);
    const int offset = header_bottom - line.top;
    HeaderShift shift{dialog, line.top, offset};
    EnumChildWindows(dialog, shift_header_content, reinterpret_cast<LPARAM>(&shift));
    check(shift.status);
    RECT window{};
    check_win32(GetWindowRect(dialog, &window));
    check_win32(SetWindowPos(dialog, nullptr, 0, 0, window.right - window.left,
        window.bottom - window.top + offset, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE));
}

void capture_layout(HWND dialog, DialogFrame& frame)
{
    const int dpi = static_cast<int>(GetDpiForWindow(dialog));
    RECT bounds{};
    check_win32(GetClientRect(dialog, &bounds));
    frame.client = {MulDiv(bounds.right, 96, dpi), MulDiv(bounds.bottom, 96, dpi)};
    check_win32(GetWindowRect(dialog, &bounds));
    frame.minimum = {MulDiv(bounds.right - bounds.left, 96, dpi),
        MulDiv(bounds.bottom - bounds.top, 96, dpi)};
    for (const auto& anchor : frame.anchors)
    {
        const HWND control = GetDlgItem(dialog, anchor.control);
        check_win32(GetWindowRect(control, &bounds));
        MapWindowPoints(nullptr, dialog, reinterpret_cast<POINT*>(&bounds), 2);
        bounds = {MulDiv(bounds.left, 96, dpi), MulDiv(bounds.top, 96, dpi),
            MulDiv(bounds.right, 96, dpi), MulDiv(bounds.bottom, 96, dpi)};
        frame.controls.push_back({control, bounds, anchor});
    }
    frame.ready = true;
}

void resize_controls(HWND dialog, const DialogFrame& frame) noexcept
{
    if (!frame.ready) return;
    const int dpi = static_cast<int>(GetDpiForWindow(dialog));
    RECT client{};
    GetClientRect(dialog, &client);
    const int extra_x = client.right - MulDiv(frame.client.cx, dpi, 96);
    const int extra_y = client.bottom - MulDiv(frame.client.cy, dpi, 96);
    for (const auto& control : frame.controls)
    {
        const auto& bounds = control.bounds;
        const auto& anchor = control.anchor;
        SetWindowPos(control.window, nullptr,
            MulDiv(bounds.left, dpi, 96) + (anchor.move_x ? extra_x : 0),
            MulDiv(bounds.top, dpi, 96) + (anchor.move_y ? extra_y : 0),
            MulDiv(bounds.right - bounds.left, dpi, 96) + (anchor.stretch_x ? extra_x : 0),
            MulDiv(bounds.bottom - bounds.top, dpi, 96) + (anchor.stretch_y ? extra_y : 0),
            SWP_NOZORDER | SWP_NOACTIVATE);
    }
}

INT_PTR dialog_bridge_impl(HWND dialog, UINT message, WPARAM first, LPARAM second)
{
    auto* frame = reinterpret_cast<DialogFrame*>(GetPropW(dialog, FrameProperty));
    if (message == WM_INITDIALOG)
    {
        frame = reinterpret_cast<DialogFrame*>(second);
        const auto status = guarded([&] {
            check_win32(SetPropW(dialog, FrameProperty, frame));
            check_win32(SetWindowTextW(dialog, text(frame->resource).c_str()));
            HRESULT translated{S_OK};
            EnumChildWindows(dialog, translate_control, reinterpret_cast<LPARAM>(&translated));
            check(translated);
            const bool close = frame->resource == IDD_THIS_PC_ENTRIES || frame->resource == IDD_ABOUT;
            SetDlgItemTextW(dialog, IDCANCEL, text(close ? IDS_CLOSE : IDS_CANCEL).c_str());
            if (GetDlgItem(dialog, IDOK))
                SetDlgItemTextW(dialog, IDOK, text(IDS_APPLY).c_str());
            const auto heading = frame->resource == IDD_ABOUT ? std::wstring(identity::launcher_name) : text(frame->resource);
            SetDlgItemTextW(dialog, IDC_DIALOG_HEADING, heading.c_str());
            if (!frame->embedded) place_dialog(dialog, frame->origin);
            fit_header(dialog, frame->resource);
            if (!frame->embedded) place_dialog(dialog, frame->origin);
            if (frame->embedded)
            {
                for (const int id : {IDCANCEL, IDOK, IDC_SHOW_ABOUT})
                {
                    const HWND control = GetDlgItem(dialog, id);
                    ShowWindow(control, SW_HIDE);
                    EnableWindow(control, FALSE);
                }
            }
            capture_layout(dialog, *frame);
            update_heading(dialog, *frame);
            return S_OK;
        });
        if (FAILED(status))
        {
            frame->failure = status;
            if (!frame->embedded) EndDialog(dialog, IDCANCEL);
            return TRUE;
        }
        return frame->callback(dialog, message, first, frame->parameter);
    }
    if (!frame) return FALSE;
    if (frame->embedded && message == WM_COMMAND && (LOWORD(first) == IDOK || LOWORD(first) == IDCANCEL))
    {
        SendMessageW(GetAncestor(dialog, GA_ROOT), message, first, second);
        return TRUE;
    }
    if (message == WM_GETMINMAXINFO && frame->ready)
    {
        const int dpi = static_cast<int>(GetDpiForWindow(dialog));
        auto* limits = reinterpret_cast<MINMAXINFO*>(second);
        limits->ptMinTrackSize = {MulDiv(frame->minimum.cx, dpi, 96), MulDiv(frame->minimum.cy, dpi, 96)};
        return TRUE;
    }
    if (message == WM_SIZE) resize_controls(dialog, *frame);
    if (message == WM_DPICHANGED || message == WM_DPICHANGED_AFTERPARENT)
        PostMessageW(dialog, RefreshDpi, 0, 0);
    if (message == RefreshDpi)
    {
        update_heading(dialog, *frame);
        resize_controls(dialog, *frame);
    }
    if (message == WM_CTLCOLORDLG)
        return reinterpret_cast<INT_PTR>(GetSysColorBrush(COLOR_WINDOW));
    if (message == WM_CTLCOLORSTATIC)
    {
        wchar_t name[32]{};
        GetClassNameW(reinterpret_cast<HWND>(second), name, ARRAYSIZE(name));
        if (wcscmp(name, L"Static") == 0 || wcscmp(name, WC_LINK) == 0)
        {
            SetTextColor(reinterpret_cast<HDC>(first), GetSysColor(COLOR_WINDOWTEXT));
            SetBkColor(reinterpret_cast<HDC>(first), GetSysColor(COLOR_WINDOW));
            if (wcscmp(name, WC_LINK) == 0) SetBkMode(reinterpret_cast<HDC>(first), TRANSPARENT);
            return reinterpret_cast<INT_PTR>(GetSysColorBrush(COLOR_WINDOW));
        }
    }
    if (message == WM_COMMAND && LOWORD(first) == IDC_SHOW_ABOUT)
    {
        const auto status = guarded([&] { show_about_dialog(dialog); return S_OK; });
        if (FAILED(status)) report_error(dialog, status, IDS_ERROR_ABOUT);
        return TRUE;
    }
    const auto result = frame->callback(dialog, message, first, second);
    if (message == WM_NCDESTROY) RemovePropW(dialog, FrameProperty);
    return result;
}

INT_PTR CALLBACK dialog_bridge(HWND dialog, UINT message, WPARAM first, LPARAM second) noexcept
{
    INT_PTR result{};
    const auto status = guarded([&] {
        result = dialog_bridge_impl(dialog, message, first, second);
        return S_OK;
    });
    if (FAILED(status))
    {
        auto* frame = reinterpret_cast<DialogFrame*>(GetPropW(dialog, FrameProperty));
        if (!frame && message == WM_INITDIALOG) frame = reinterpret_cast<DialogFrame*>(second);
        if (frame) frame->failure = status;
        if (!frame || !frame->embedded) EndDialog(dialog, IDCANCEL);
        return TRUE;
    }
    return result;
}

INT_PTR CALLBACK about_dialog(HWND dialog, UINT message, WPARAM first, LPARAM second) noexcept
{
    if (message == WM_INITDIALOG) return TRUE;
    if (message == WM_COMMAND && LOWORD(first) == IDCANCEL)
    {
        EndDialog(dialog, IDCANCEL);
        return TRUE;
    }
    if (message != WM_NOTIFY) return FALSE;
    const auto* link = reinterpret_cast<NMLINK*>(second);
    if (link->hdr.code != NM_CLICK && link->hdr.code != NM_RETURN) return FALSE;
    const auto status = guarded([&] {
        // Only these two built-in destinations are actionable, never link text.
        const wchar_t* destination = nullptr;
        if (wcscmp(link->item.szID, L"website") == 0) destination = identity::website;
        if (wcscmp(link->item.szID, L"github") == 0) destination = identity::repository;
        return destination ? launch_process(dialog, destination, L"") : E_INVALIDARG;
    });
    if (FAILED(status)) report_error(dialog, status, IDS_ERROR_ABOUT);
    return TRUE;
}

HRESULT CALLBACK message_dialog(HWND dialog, UINT message, WPARAM, LPARAM, LONG_PTR data) noexcept
{
    if (message != TDN_CREATED) return S_OK;
    return guarded([&] {
        auto& frame = *reinterpret_cast<DialogFrame*>(data);
        update_heading(dialog, frame);
        return S_OK;
    });
}
}

NativeUiScope::NativeUiScope()
{
    ACTCTXW context{sizeof(context)};
    context.dwFlags = ACTCTX_FLAG_HMODULE_VALID | ACTCTX_FLAG_RESOURCE_NAME_VALID;
    context.hModule = current_module();
#ifdef MCOS_EXECUTABLE
    context.lpResourceName = MAKEINTRESOURCEW(1);
#else
    context.lpResourceName = MAKEINTRESOURCEW(2);
#endif
    activation_ = CreateActCtxW(&context);
    if (activation_ == INVALID_HANDLE_VALUE) check(HRESULT_FROM_WIN32(GetLastError()));
    if (!ActivateActCtx(activation_, &cookie_))
    {
        const DWORD error = GetLastError();
        ReleaseActCtx(activation_);
        check(HRESULT_FROM_WIN32(error));
    }
    previous_dpi_ = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    if (!previous_dpi_)
    {
        const DWORD error = GetLastError();
        DeactivateActCtx(0, cookie_);
        ReleaseActCtx(activation_);
        check(HRESULT_FROM_WIN32(error));
    }
    INITCOMMONCONTROLSEX controls{sizeof(controls), ICC_STANDARD_CLASSES | ICC_LISTVIEW_CLASSES | ICC_LINK_CLASS | ICC_TAB_CLASSES};
    if (!InitCommonControlsEx(&controls))
    {
        const DWORD error = GetLastError();
        SetThreadDpiAwarenessContext(previous_dpi_);
        DeactivateActCtx(0, cookie_);
        ReleaseActCtx(activation_);
        check(HRESULT_FROM_WIN32(error ? error : ERROR_DLL_INIT_FAILED));
    }
}

NativeUiScope::~NativeUiScope()
{
    SetThreadDpiAwarenessContext(previous_dpi_);
    DeactivateActCtx(0, cookie_);
    ReleaseActCtx(activation_);
}

RECT dialog_origin(HWND owner)
{
    NativeUiScope ui;
    RECT origin{};
    if (usable_owner(owner))
    {
        const HWND root = GetAncestor(owner, GA_ROOT);
        if (GetWindowRect(root ? root : owner, &origin)) return origin;
    }
    POINT pointer{};
    check_win32(GetCursorPos(&pointer));
    MONITORINFO info{sizeof(info)};
    check_win32(GetMonitorInfoW(MonitorFromPoint(pointer, MONITOR_DEFAULTTONEAREST), &info));
    return info.rcWork;
}

std::wstring dialog_origin_arguments(const RECT& origin)
{
    return L" --origin " + std::to_wstring(origin.left) + L" " + std::to_wstring(origin.top) +
        L" " + std::to_wstring(origin.right) + L" " + std::to_wstring(origin.bottom);
}

INT_PTR show_native_dialog(UINT resource, HWND owner, DLGPROC callback, LPARAM state,
    std::span<const DialogAnchor> anchors, const RECT* origin)
{
    NativeUiScope ui;
    const RECT source = origin ? *origin : dialog_origin(owner);
    PositionOwner positioning;
    if (!usable_owner(owner))
    {
        positioning.window = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, L"STATIC", L"",
            WS_POPUP, source.left, source.top, source.right - source.left,
            source.bottom - source.top, nullptr, nullptr, current_module(), nullptr);
        check_win32(positioning.window != nullptr);
        owner = positioning.window;
    }
    DialogFrame frame{resource, callback, state, source, {anchors.begin(), anchors.end()}};
    const auto result = DialogBoxParamW(current_module(), MAKEINTRESOURCEW(resource), owner,
        dialog_bridge, reinterpret_cast<LPARAM>(&frame));
    check(frame.failure);
    if (result == -1) check(HRESULT_FROM_WIN32(GetLastError()));
    return result;
}

void show_about_dialog(HWND owner)
{
    show_native_dialog(IDD_ABOUT, owner, about_dialog, 0);
}

void fail_native_dialog(HWND dialog, HRESULT error) noexcept
{
    auto* frame = reinterpret_cast<DialogFrame*>(GetPropW(dialog, FrameProperty));
    if (!frame) return;
    frame->failure = error;
    if (!frame->embedded) EndDialog(dialog, IDCANCEL);
}

struct NativeDialogPage::State
{
    DialogFrame frame{};
    std::shared_ptr<void> controller;
    HWND window{};
    ~State() { if (window && IsWindow(window)) DestroyWindow(window); }
};

NativeDialogPage::NativeDialogPage() : state_(std::make_unique<State>()) {}
NativeDialogPage::~NativeDialogPage() = default;
HWND NativeDialogPage::window() const noexcept { return state_->window; }

std::unique_ptr<NativeDialogPage> NativeDialogPage::create(UINT resource, HWND parent,
    DLGPROC callback, LPARAM parameter, std::span<const DialogAnchor> anchors,
    std::shared_ptr<void> controller)
{
    auto page = std::unique_ptr<NativeDialogPage>(new NativeDialogPage);
    auto& state = *page->state_;
    state.controller = std::move(controller);
    state.frame.resource = resource;
    state.frame.callback = callback;
    state.frame.parameter = parameter;
    state.frame.anchors.assign(anchors.begin(), anchors.end());
    state.frame.embedded = true;

    // All project templates are DIALOGEX. Clone the immutable resource, changing
    // only the documented extended header's window styles for a tab child.
    const auto module = current_module();
    const HRSRC found = FindResourceW(module, MAKEINTRESOURCEW(resource), RT_DIALOG);
    check_win32(found != nullptr);
    const DWORD bytes = SizeofResource(module, found);
    const HGLOBAL loaded = LoadResource(module, found);
    const void* source = LockResource(loaded);
    check_win32(source != nullptr);
    if (bytes < 26) check(E_INVALIDARG);
    std::vector<DWORD> storage((bytes + sizeof(DWORD) - 1) / sizeof(DWORD));
    std::memcpy(storage.data(), source, bytes);
    const auto* words = reinterpret_cast<const WORD*>(storage.data());
    if (words[0] != 1 || words[1] != 0xffff) check(E_INVALIDARG);
    storage[2] = WS_EX_CONTROLPARENT;
    storage[3] = DS_SETFONT | DS_CONTROL | WS_CHILD | WS_CLIPSIBLINGS;
    state.window = CreateDialogIndirectParamW(module,
        reinterpret_cast<const DLGTEMPLATE*>(storage.data()), parent,
        dialog_bridge, reinterpret_cast<LPARAM>(&state.frame));
    check(state.frame.failure);
    check_win32(state.window != nullptr);
    return page;
}

std::unique_ptr<NativeDialogPage> create_about_page(HWND parent)
{
    const DialogAnchor anchors[]{
        {IDC_DIALOG_HEADING, true}, {IDC_ABOUT_DESCRIPTION, true},
        {IDC_DIALOG_HEADER_LINE, true}, {IDC_ABOUT_CREDIT, true},
        {IDC_DIALOG_FOOTER_LINE, true, false, false, true}};
    return NativeDialogPage::create(IDD_ABOUT, parent, about_dialog, 0, anchors);
}

void show_native_message(HWND owner, const std::wstring& message)
{
    NativeUiScope ui;
    PositionOwner positioning;
    if (!usable_owner(owner))
    {
        const RECT source = dialog_origin(nullptr);
        positioning.window = CreateWindowExW(WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, L"STATIC", L"",
            WS_POPUP, source.left, source.top, source.right - source.left,
            source.bottom - source.top, nullptr, nullptr, current_module(), nullptr);
        check_win32(positioning.window != nullptr);
        owner = positioning.window;
    }
    DialogFrame frame{};
    const int pixels = GetSystemMetricsForDpi(SM_CXICON, GetDpiForWindow(owner));
    const auto icon = reinterpret_cast<HICON>(LoadImageW(current_module(), MAKEINTRESOURCEW(IDI_APP),
        IMAGE_ICON, pixels, pixels, LR_SHARED));
    TASKDIALOGCONFIG configuration{sizeof(configuration)};
    configuration.hwndParent = owner;
    configuration.dwFlags = TDF_POSITION_RELATIVE_TO_WINDOW | TDF_USE_HICON_MAIN | TDF_ALLOW_DIALOG_CANCELLATION;
    configuration.dwCommonButtons = TDCBF_OK_BUTTON;
    configuration.pszWindowTitle = identity::launcher_name;
    configuration.pszContent = message.c_str();
    configuration.hMainIcon = icon;
    configuration.pfCallback = message_dialog;
    configuration.lpCallbackData = reinterpret_cast<LONG_PTR>(&frame);
    check(TaskDialogIndirect(&configuration, nullptr, nullptr, nullptr));
}
}
