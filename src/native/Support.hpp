// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; native launcher implementation; 2026-10-11.
 * Modified with AI: OpenAI Codex; shared process and dialog helpers; 2026-10-17.
 * Human guidance: Umberto Giacobbi; standard Shell view, native menus and MSI.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#pragma once
#include <Windows.h>
#include <ShlObj.h>
#include <Shlwapi.h>
#include <Shellapi.h>
#include <wrl/client.h>
#include <memory>
#include <string>
#include <string_view>
#include <filesystem>
#include <new>

namespace mcos
{
template<class T> using ComPtr = Microsoft::WRL::ComPtr<T>;
struct TaskMemFree { void operator()(void* value) const noexcept { CoTaskMemFree(value); } };
struct PidlFree
{
    using pointer = PIDLIST_ABSOLUTE;
    void operator()(pointer value) const noexcept { CoTaskMemFree(value); }
};
using Pidl = std::unique_ptr<ITEMIDLIST, PidlFree>;
using TaskString = std::unique_ptr<wchar_t, TaskMemFree>;
struct HResultError { HRESULT value; };
inline void check(HRESULT value) { if (FAILED(value)) throw HResultError{value}; }
inline void check_win32(BOOL success) { if (!success) check(HRESULT_FROM_WIN32(GetLastError())); }

// COM callbacks use this boundary so C++ exceptions become HRESULT failures.
template<class F> HRESULT guarded(F&& action) noexcept
{
    try { return action(); }
    catch (const HResultError& error) { return error.value; }
    catch (const std::bad_alloc&) { return E_OUTOFMEMORY; }
    catch (const std::filesystem::filesystem_error& error)
    { return HRESULT_FROM_WIN32(static_cast<DWORD>(error.code().value())); }
    catch (...) { return E_FAIL; }
}

inline Pidl parse_pidl(const std::wstring& path)
{
    PIDLIST_ABSOLUTE raw = nullptr;
    check(SHParseDisplayName(path.c_str(), nullptr, &raw, 0, nullptr));
    return Pidl(raw);
}

inline ComPtr<IShellItem> shell_item(PCIDLIST_ABSOLUTE pidl)
{
    ComPtr<IShellItem> item;
    check(SHCreateItemFromIDList(pidl, IID_PPV_ARGS(&item)));
    return item;
}

inline std::wstring display_name(IShellItem* item, SIGDN kind = SIGDN_NORMALDISPLAY)
{
    PWSTR raw = nullptr;
    check(item->GetDisplayName(kind, &raw));
    TaskString owned(raw);
    return raw;
}

std::wstring text(UINT resource_id);

// The module containing these helpers: the Shell DLL in Explorer, or the EXE.
HMODULE current_module();
std::filesystem::path module_path(HMODULE module);

// Quotes one argument for CommandLineToArgvW, including trailing backslashes.
std::wstring quote_command_line_argument(std::wstring_view value);

enum class LaunchMode { normal, elevated };

// Starts without waiting. S_FALSE means UAC was cancelled; the process was not
// started. S_OK confirms launch only, not completion of the child's work.
HRESULT launch_process(HWND owner, const std::filesystem::path& executable,
    const std::wstring& arguments, LaunchMode mode = LaunchMode::normal);

// Returns the control's complete text, without its terminating null character.
std::wstring dialog_text(HWND dialog, UINT control);

void report_error(HWND owner, HRESULT error, UINT operation) noexcept;
void notify_launcher() noexcept;
}
