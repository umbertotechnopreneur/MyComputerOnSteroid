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

#include "native/Support.hpp"
#include "native/ThisPcEntries.hpp"
#include "core/Catalog.hpp"
#include "ProjectIdentity.hpp"
#include "../resources/resource.h"

int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int)
{
    const HRESULT initialized=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    if(FAILED(initialized)) return 1;
    int count=0;
    auto* arguments=CommandLineToArgvW(GetCommandLineW(),&count);
    const auto status=mcos::guarded([&]{
        if(!arguments) return E_OUTOFMEMORY;
        if(count==1 || (count==2 && wcscmp(arguments[1],L"--open")==0))
        {
            const auto launched=reinterpret_cast<INT_PTR>(ShellExecuteW(nullptr,L"open",L"explorer.exe",
                mcos::identity::parsing_name,nullptr,SW_SHOWNORMAL));
            return launched>32 ? S_OK : HRESULT_FROM_WIN32(ERROR_OPEN_FAILED);
        }
        if(count==2 && wcscmp(arguments[1],L"--manage-this-pc")==0)
            return mcos::launch_this_pc_entry_manager();
        if(count>=3 && wcscmp(arguments[1],L"--add-this-pc")==0)
        {
            std::vector<std::wstring> folders;
            folders.reserve(static_cast<size_t>(count-2));
            for(int i=2;i<count;++i) folders.emplace_back(arguments[i]);
            return mcos::add_this_pc_folders(folders);
        }
        mcos::Section section=static_cast<mcos::Section>(0);
        if(wcscmp(arguments[1],L"--folder")==0) section=mcos::Section::folders;
        else if(wcscmp(arguments[1],L"--app")==0) section=mcos::Section::apps;
        else if(wcscmp(arguments[1],L"--file")==0) section=mcos::Section::files;
        else if(wcscmp(arguments[1],L"--add")!=0) return E_INVALIDARG;
        if(count<3) return E_INVALIDARG;
        for(int i=2;i<count;++i)
            mcos::add_favorite(mcos::shell_item(mcos::parse_pidl(arguments[i]).get()).Get(),section);
        return S_OK;
    });
    LocalFree(arguments);
    if(FAILED(status)) mcos::report_error(GetDesktopWindow(),status,IDS_ERROR_ADD);
    CoUninitialize();
    return FAILED(status) ? 1 : 0;
}
