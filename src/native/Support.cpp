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

#include "Support.hpp"
#include "../ProjectIdentity.hpp"
#include "../core/Catalog.hpp"
#include "../../resources/resource.h"
#include <fstream>
#include <sstream>
#include <iomanip>

namespace mcos
{
HMODULE current_module()
{
    HMODULE module = nullptr;
    const DWORD flags = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
    check_win32(GetModuleHandleExW(flags,
        reinterpret_cast<LPCWSTR>(&current_module), &module));
    return module;
}

std::filesystem::path module_path(HMODULE module)
{
    std::wstring path(32768, L'\0');
    const DWORD length = GetModuleFileNameW(module, path.data(), static_cast<DWORD>(path.size()));
    if (!length) check(HRESULT_FROM_WIN32(GetLastError()));
    if (length >= path.size()) check(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));

    path.resize(length);
    return path;
}

std::wstring quote_command_line_argument(std::wstring_view value)
{
    std::wstring quoted{L"\""};
    size_t backslashes = 0;

    for (const wchar_t character : value)
    {
        if (character == L'\\')
        {
            ++backslashes;
            continue;
        }

        // Before a quote, double literal backslashes and escape the quote.
        const size_t slash_count = character == L'"' ? backslashes * 2 + 1 : backslashes;
        quoted.append(slash_count, L'\\');
        quoted.push_back(character);
        backslashes = 0;
    }

    // Backslashes before the closing quote also need doubling (for C:\, etc.).
    quoted.append(backslashes * 2, L'\\');
    quoted.push_back(L'"');
    return quoted;
}

HRESULT launch_process(HWND owner, const std::filesystem::path& executable,
    const std::wstring& arguments, LaunchMode mode)
{
    const wchar_t* verb = mode == LaunchMode::elevated ? L"runas" : L"open";
    const auto launched = reinterpret_cast<INT_PTR>(ShellExecuteW(
        owner, verb, executable.c_str(), arguments.c_str(), nullptr, SW_SHOWNORMAL));
    if (launched > 32) return S_OK;

    const DWORD error = GetLastError();
    if (error == ERROR_CANCELLED) return S_FALSE;
    return HRESULT_FROM_WIN32(error == ERROR_SUCCESS ? ERROR_OPEN_FAILED : error);
}

std::wstring dialog_text(HWND dialog, UINT control)
{
    const int length = GetWindowTextLengthW(GetDlgItem(dialog, control));
    std::wstring value(static_cast<size_t>(length) + 1, L'\0');
    GetDlgItemTextW(dialog, control, value.data(), length + 1);
    value.resize(static_cast<size_t>(length));
    return value;
}

std::wstring text(UINT id)
{
    wchar_t* raw = nullptr;
    const int length = LoadStringW(current_module(), id, reinterpret_cast<LPWSTR>(&raw), 0);
    if (length <= 0) check(HRESULT_FROM_WIN32(ERROR_RESOURCE_NAME_NOT_FOUND));
    return std::wstring(raw, static_cast<size_t>(length));
}

void report_error(HWND owner, HRESULT error, UINT operation) noexcept
{
    guarded([&] {
        std::wostringstream message;
        message << text(operation) << L"\nHRESULT 0x" << std::hex << std::uppercase
                << static_cast<unsigned long>(error);
        // Store only error codes and operation labels, never target filenames.
        guarded([&] {
            const auto logs = data_root() / L"logs";
            std::filesystem::create_directories(logs);
            std::wofstream log(logs / L"launcher.log", std::ios::app);
            SYSTEMTIME now{}; GetLocalTime(&now);
            log << now.wYear << L"-" << now.wMonth << L"-" << now.wDay << L" "
                << now.wHour << L":" << now.wMinute << L" " << message.str() << L"\n";
            return S_OK;
        }); // A failed log write must not suppress the visible error.
        if (owner) MessageBoxW(owner, message.str().c_str(), identity::launcher_name, MB_OK | MB_ICONERROR);
        return S_OK;
    });
}

void notify_launcher() noexcept
{
    // This may legitimately fail in an unregistered, isolated test process.
    guarded([] {
        auto root = parse_pidl(identity::parsing_name);
        SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, root.get(), nullptr);
        return S_OK;
    });
}
}
