// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; native launcher implementation; 2026-10-11.
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
std::wstring text(UINT id)
{
    HMODULE module = nullptr;
    check_win32(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
        GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<LPCWSTR>(&identity::launcher_clsid), &module));
    wchar_t* raw = nullptr;
    const int length = LoadStringW(module, id, reinterpret_cast<LPWSTR>(&raw), 0);
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
