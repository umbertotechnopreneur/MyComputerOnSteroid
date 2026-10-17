// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; themed, DPI-aware native dialogs; 2026-10-17.
 * Human guidance: Umberto Giacobbi; clear headers and source-monitor placement.
 * Evidence: docs/VALIDATION.md; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#pragma once
#include "Support.hpp"
#include <span>
#include <memory>

namespace mcos
{
// The activation context and DPI mode are limited to the calling UI scope.
// In Explorer, neither the host's process DPI mode nor other threads change.
class NativeUiScope final
{
public:
    NativeUiScope();
    ~NativeUiScope();
    NativeUiScope(const NativeUiScope&) = delete;
    NativeUiScope& operator=(const NativeUiScope&) = delete;

private:
    HANDLE activation_{INVALID_HANDLE_VALUE};
    ULONG_PTR cookie_{};
    DPI_AWARENESS_CONTEXT previous_dpi_{};
};

struct DialogAnchor
{
    UINT control;
    bool stretch_x{};
    bool move_x{};
    bool stretch_y{};
    bool move_y{};
};

// Captures the clicked top-level window's bounds, or the pointer's monitor.
// The rectangle can be passed across an elevated relaunch without a stale HWND.
RECT dialog_origin(HWND owner);
std::wstring dialog_origin_arguments(const RECT& origin);

// Owns only presentation state. The supplied callback retains DWLP_USER and its
// normal WM_INITDIALOG parameter. Fonts/layout scale through PMv2 dialog support.
INT_PTR show_native_dialog(UINT resource, HWND owner, DLGPROC callback, LPARAM state,
    std::span<const DialogAnchor> anchors = {}, const RECT* origin = nullptr);

// Child pages share the modal host's message loop and DPI context. Their frame
// and optional controller state outlive the HWND and are released together.
class NativeDialogPage final
{
public:
    struct State;
    ~NativeDialogPage();
    NativeDialogPage(const NativeDialogPage&) = delete;
    NativeDialogPage& operator=(const NativeDialogPage&) = delete;
    HWND window() const noexcept;
    static std::unique_ptr<NativeDialogPage> create(UINT resource, HWND parent,
        DLGPROC callback, LPARAM parameter, std::span<const DialogAnchor> anchors = {},
        std::shared_ptr<void> controller = {});
private:
    NativeDialogPage();
    std::unique_ptr<State> state_;
};

// Page callbacks write an HRESULT through LPARAM; no persistence or closing.
inline constexpr UINT ReadDialogPage = WM_APP + 20;
void fail_native_dialog(HWND dialog, HRESULT error) noexcept;
std::unique_ptr<NativeDialogPage> create_about_page(HWND parent);

void show_about_dialog(HWND owner);
// Standard Windows message dialog, with the same monitor and application icon.
void show_native_message(HWND owner, const std::wstring& message);
}
