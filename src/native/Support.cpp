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
#include "Dialog.hpp"
#include "../ProjectIdentity.hpp"
#include "../core/Catalog.hpp"
#include "../../resources/resource.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>

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

LANGID ui_language()
{
    ULONG count{};
    ULONG characters{};
    check_win32(GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count, nullptr, &characters));
    std::vector<wchar_t> names(characters);
    check_win32(GetUserPreferredUILanguages(MUI_LANGUAGE_NAME, &count, names.data(), &characters));
    for (const wchar_t* name = names.data(); *name; name += wcslen(name) + 1)
    {
        const std::wstring_view language(name);
        const auto starts = [&](std::wstring_view prefix) {
            return language.size() >= prefix.size() &&
                _wcsnicmp(language.data(), prefix.data(), prefix.size()) == 0 &&
                (language.size() == prefix.size() || language[prefix.size()] == L'-');
        };
        if (starts(L"it")) return MAKELANGID(LANG_ITALIAN, SUBLANG_DEFAULT);
        if (starts(L"en")) return MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
        if (starts(L"vi")) return MAKELANGID(LANG_VIETNAMESE, SUBLANG_DEFAULT);
        if (starts(L"fr")) return MAKELANGID(LANG_FRENCH, SUBLANG_FRENCH);
        if (starts(L"de")) return MAKELANGID(LANG_GERMAN, SUBLANG_GERMAN);
        if (starts(L"es")) return MAKELANGID(LANG_SPANISH, SUBLANG_SPANISH_MODERN);
        if (starts(L"yue") || starts(L"zh-HK") || starts(L"zh-MO"))
            return MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_HONGKONG);
        if (starts(L"zh")) return MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
    }
    return MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
}

std::wstring text(UINT id)
{
    const auto module = current_module();
    const auto resource = FindResourceExW(module, RT_STRING, MAKEINTRESOURCEW(id / 16 + 1), ui_language());
    if (!resource) check(HRESULT_FROM_WIN32(ERROR_RESOURCE_NAME_NOT_FOUND));
    const auto loaded = LoadResource(module, resource);
    if (!loaded) check(HRESULT_FROM_WIN32(GetLastError()));
    const auto* value = static_cast<const wchar_t*>(LockResource(loaded));
    if (!value) check(HRESULT_FROM_WIN32(ERROR_RESOURCE_DATA_NOT_FOUND));
    const auto* end = value + SizeofResource(module, resource) / sizeof(wchar_t);
    for (UINT index = 0; index < id % 16; ++index)
    {
        if (value >= end || value + 1 + *value > end) check(E_UNEXPECTED);
        value += 1 + *value;
    }
    if (value >= end || !*value || value + 1 + *value > end)
        check(HRESULT_FROM_WIN32(ERROR_RESOURCE_NAME_NOT_FOUND));
    return std::wstring(value + 1, *value);
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
        if (owner)
        {
            show_native_message(owner, message.str());
        }
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
