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
#include "native/Dialog.hpp"
#include "native/ThisPcEntries.hpp"
#include "native/Configuration.hpp"
#include "core/Catalog.hpp"
#include "ProjectIdentity.hpp"
#include "../resources/resource.h"
#include <cerrno>
#include <array>

int WINAPI wWinMain(HINSTANCE,HINSTANCE,PWSTR,int)
{
    const HRESULT initialized=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    if(FAILED(initialized)) return 1;
    int count=0;
    auto* arguments=CommandLineToArgvW(GetCommandLineW(),&count);
    bool worker = false;
    const auto status=mcos::guarded([&]{
        if(!arguments) return E_OUTOFMEMORY;
        if(count==1 || (count==2 && wcscmp(arguments[1],L"--open")==0))
        {
            const auto launched=reinterpret_cast<INT_PTR>(ShellExecuteW(nullptr,L"open",L"explorer.exe",
                mcos::identity::parsing_name,nullptr,SW_SHOWNORMAL));
            return launched>32 ? S_OK : HRESULT_FROM_WIN32(ERROR_OPEN_FAILED);
        }
        if(count==2 && wcscmp(arguments[1],L"--about")==0)
        {
            mcos::show_about_dialog(nullptr);
            return S_OK;
        }
        if(count >= 2 && wcscmp(arguments[1], L"--this-pc-worker") == 0)
        {
            worker = true;
            std::vector<std::wstring> values;
            for (int index = 2; index < count; ++index) values.emplace_back(arguments[index]);
            return mcos::run_this_pc_worker(values);
        }
        if((count==2 || count==7) &&
            (wcscmp(arguments[1],L"--configure")==0 || wcscmp(arguments[1],L"--manage-this-pc")==0))
        {
            mcos::NativeUiScope ui;
            RECT origin = mcos::dialog_origin(nullptr);
            if(count==7)
            {
                if(wcscmp(arguments[2],L"--origin")!=0) return E_INVALIDARG;
                std::array<LONG,4> coordinates{};
                for(size_t index=0;index<coordinates.size();++index)
                {
                    wchar_t* end=nullptr;
                    errno=0;
                    coordinates[index]=wcstol(arguments[index+3],&end,10);
                    if(errno || end==arguments[index+3] || *end) return E_INVALIDARG;
                    if(coordinates[index] < -100000 || coordinates[index] > 100000) return E_INVALIDARG;
                }
                origin={coordinates[0],coordinates[1],coordinates[2],coordinates[3]};
                if(origin.right<=origin.left || origin.bottom<=origin.top) return E_INVALIDARG;
            }
            mcos::show_configuration(origin);
            return S_OK;
        }
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
    if(FAILED(status) && !worker) mcos::report_error(GetDesktopWindow(),status,IDS_ERROR_CONFIGURATION);
    CoUninitialize();
    if (worker) return static_cast<int>(status);
    return FAILED(status) ? 1 : 0;
}
