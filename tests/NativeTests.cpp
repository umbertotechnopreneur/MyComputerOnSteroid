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

#include "../src/core/Catalog.hpp"
#include "../src/core/Recents.hpp"
#include "../src/core/ViewSettings.hpp"
#include "../src/shell/ItemId.hpp"
#include "../src/ProjectIdentity.hpp"
#include "../resources/resource.h"
#include <Propkey.h>
#include <Propsys.h>
#include <fstream>
#include <iostream>
#include <bit>
#include <array>
#include <algorithm>
#include <thread>
using namespace mcos;

namespace
{
void require(bool condition,const char* message)
{
    if(!condition) { std::cerr<<"FAIL: "<<message<<"\n"; throw HResultError{E_FAIL}; }
    std::cout<<"PASS: "<<message<<"\n";
}

std::string contents(const std::filesystem::path& path)
{
    std::ifstream input(path,std::ios::binary);
    return {std::istreambuf_iterator<char>(input),std::istreambuf_iterator<char>()};
}
void write_test_icon(const std::filesystem::path& path)
{
    std::vector<unsigned char> bytes;
    const auto word=[&](unsigned value) { bytes.push_back(static_cast<unsigned char>(value)); bytes.push_back(static_cast<unsigned char>(value>>8)); };
    const auto dword=[&](unsigned value) { word(value & 0xffff); word(value>>16); };
    word(0); word(1); word(1); // ICONDIR
    bytes.insert(bytes.end(),{16,16,0,0}); word(1); word(32); dword(40+1024+64); dword(22);
    dword(40); dword(16); dword(32); word(1); word(32); dword(0); dword(1024+64);
    dword(0); dword(0); dword(0); dword(0);
    for(int pixel=0;pixel<256;++pixel) dword(0xffdd8844); // Opaque color, no external artwork.
    bytes.resize(bytes.size()+64,0);
    std::ofstream output(path,std::ios::binary);
    output.write(reinterpret_cast<const char*>(bytes.data()),static_cast<std::streamsize>(bytes.size()));
}
void verify_launcher_icons(HMENU popup,IContextMenu* menu)
{
    for(int index=0;index<GetMenuItemCount(popup);++index)
    {
        const UINT command=GetMenuItemID(popup,index);
        if(command==static_cast<UINT>(-1) || !command) continue;
        wchar_t verb[128]{};
        if(FAILED(menu->GetCommandString(command-1,GCS_VERBW,nullptr,reinterpret_cast<LPSTR>(verb),ARRAYSIZE(verb))) ||
            wcsncmp(verb,L"mcos.",5)!=0) continue;
        MENUITEMINFOW item{sizeof(item)}; item.fMask=MIIM_BITMAP;
        if(!GetMenuItemInfoW(popup,command,FALSE,&item) || !item.hbmpItem)
        {
            std::wcerr<<L"Missing native menu bitmap for "<<verb<<L" at position "<<index<<L"\n";
            HDC dc=CreateCompatibleDC(nullptr);
            if(dc)
            {
                for(PCWSTR face:{L"Segoe Fluent Icons",L"Segoe MDL2 Assets"})
                {
                    HFONT font=CreateFontW(-16,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,
                        OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,ANTIALIASED_QUALITY,DEFAULT_PITCH,face);
                    if(!font) continue;
                    HGDIOBJ previous=SelectObject(dc,font); wchar_t actual[LF_FACESIZE]{};
                    const wchar_t characters[]{0xed1a,0xe738}; WORD glyphs[]{0xffff,0xffff};
                    GetTextFaceW(dc,ARRAYSIZE(actual),actual);
                    GetGlyphIndicesW(dc,characters,2,glyphs,GGI_MARK_NONEXISTING_GLYPHS);
                    std::wcerr<<L"Font "<<face<<L" maps to "<<actual<<L"; Hide/Remove indices "
                        <<std::hex<<glyphs[0]<<L"/"<<glyphs[1]<<std::dec<<L"\n";
                    SelectObject(dc,previous); DeleteObject(font);
                }
                DeleteDC(dc);
            }
        }
        require(GetMenuItemInfoW(popup,command,FALSE,&item) && item.hbmpItem,"launcher commands have native menu bitmaps");
        DIBSECTION image{};
        require(GetObjectW(item.hbmpItem,sizeof(image),&image)==sizeof(image) && image.dsBm.bmBits &&
            image.dsBm.bmBitsPixel==32 && image.dsBm.bmWidth>=16 && image.dsBm.bmHeight>=16,
            "menu icon has a usable native 32-bit surface");
        const auto* bits=static_cast<const DWORD*>(image.dsBm.bmBits);
        bool transparent=false,visible=false,premultiplied=true;
        for(int pixel=0;pixel<image.dsBm.bmWidth*image.dsBm.bmHeight;++pixel)
        {
            const DWORD alpha=bits[pixel]>>24;
            transparent=transparent || alpha==0; visible=visible || alpha!=0;
            premultiplied=premultiplied && (bits[pixel] & 0xff)<=alpha && ((bits[pixel]>>8) & 0xff)<=alpha &&
                ((bits[pixel]>>16) & 0xff)<=alpha;
        }
        require(premultiplied,"native menu bitmap uses premultiplied alpha");
        require(transparent && visible,"menu glyph is visible on a transparent background");
    }
}
UINT menu_command(IContextMenu* menu,PCWSTR wanted)
{
    HMENU popup=CreatePopupMenu();
    const auto status=menu->QueryContextMenu(popup,0,1,0x7fff,CMF_NORMAL);
    UINT result=0;
    if(SUCCEEDED(status))
        for(int index=0;index<GetMenuItemCount(popup);++index)
        {
            const UINT command=GetMenuItemID(popup,index);
            if(command==static_cast<UINT>(-1) || !command) continue;
            wchar_t verb[128]{};
            if(SUCCEEDED(menu->GetCommandString(command-1,GCS_VERBW,nullptr,reinterpret_cast<LPSTR>(verb),ARRAYSIZE(verb))) &&
                wcscmp(verb,wanted)==0) result=command;
        }
    if(SUCCEEDED(status)) verify_launcher_icons(popup,menu);
    DestroyMenu(popup); check(status);
    return result;
}
void invoke_numeric(IContextMenu* menu,UINT command)
{
    require(command!=0,"requested canonical menu command exists");
    CMINVOKECOMMANDINFO invoke{}; invoke.cbSize=sizeof(invoke);
    invoke.lpVerb=MAKEINTRESOURCEA(command-1); invoke.nShow=SW_HIDE;
    check(menu->InvokeCommand(&invoke));
}
void recent_tests(const std::filesystem::path& fixture,IShellFolder2* shell)
{
    const auto source=fixture/L"windows-recent";
    const auto targets=fixture/L"recent-targets";
    std::filesystem::create_directories(source); std::filesystem::create_directories(targets);
    check_win32(SetEnvironmentVariableW(L"MCOS_RECENT_DIR",source.c_str()));
    require(enumerate_recents().empty(),"recent items are opt-in by default");
    FILETIME now{}; GetSystemTimeAsFileTime(&now);
    const ULONGLONG base=(static_cast<ULONGLONG>(now.dwHighDateTime)<<32)|now.dwLowDateTime;
    unsigned ordinal=0;
    for(PCWSTR name:{L"old.pdf",L"Rapporto è.pdf",L"image.PNG",L"site.CSS"})
    {
        const auto target=targets/name;
        std::ofstream(target)<<"An original document; never delete this.";
        ComPtr<IShellLinkW> link;
        check(CoCreateInstance(CLSID_ShellLink,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&link)));
        check(link->SetPath(target.c_str()));
        ComPtr<IPersistFile> file; check(link.As(&file));
        const auto path=source/(std::wstring(name)+L".lnk"); check(file->Save(path.c_str(),TRUE));
        const ULONGLONG stamp=base+(++ordinal)*10000000ull;
        const FILETIME time{static_cast<DWORD>(stamp),static_cast<DWORD>(stamp>>32)};
        HANDLE handle=CreateFileW(path.c_str(),FILE_WRITE_ATTRIBUTES,FILE_SHARE_READ,nullptr,OPEN_EXISTING,0,nullptr);
        if(handle==INVALID_HANDLE_VALUE) check(HRESULT_FROM_WIN32(GetLastError()));
        const auto saved=SetFileTime(handle,nullptr,nullptr,&time); const auto error=GetLastError(); CloseHandle(handle);
        if(!saved) check(HRESULT_FROM_WIN32(error));
    }
    std::ofstream(source/L"broken.lnk")<<"Not a shortcut";
    ComPtr<IShellLinkW> virtual_link;
    check(CoCreateInstance(CLSID_ShellLink,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&virtual_link)));
    PIDLIST_ABSOLUTE control_panel_raw=nullptr;
    check(SHGetKnownFolderIDList(FOLDERID_ControlPanelFolder,KF_FLAG_DEFAULT,nullptr,&control_panel_raw));
    Pidl control_panel(control_panel_raw); check(virtual_link->SetIDList(control_panel.get()));
    ComPtr<IPersistFile> virtual_file; check(virtual_link.As(&virtual_file)); check(virtual_file->Save((source/L"virtual.lnk").c_str(),TRUE));
    RecentSettings settings; settings.enabled=true; save_recent_settings(settings);
    auto entries=enumerate_recents();
    require(entries.size()==4,"Windows Recent documents load; malformed and virtual entries are skipped");
    require(recent_info(entries.front()).name==L"site.CSS","recent entries sort newest first by source shortcut date");
    const auto date=recent_info(entries.front()).updated;
    require(((static_cast<ULONGLONG>(date.dwHighDateTime)<<32)|date.dwLowDateTime)==base+40000000ull,
        "recent date is the Windows shortcut timestamp, not the target modification date");
    const auto original_pdf=contents(targets/L"Rapporto è.pdf");
    const auto windows_pdf=contents(source/L"Rapporto è.pdf.lnk");
    std::array<HRESULT,2> concurrent{};
    std::array<std::jthread,2> workers;
    for(size_t index=0;index<workers.size();++index)
        workers[index]=std::jthread([&,index] {
            const auto initialized=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
            concurrent[index]=FAILED(initialized) ? initialized : guarded([&] {
                for(int repeat=0;repeat<12;++repeat)
                {
                    const auto catalog=enumerate_catalog();
                    if(std::count_if(catalog.begin(),catalog.end(),[](const auto& item) { return item.section==Section::recents; })!=4 ||
                        std::none_of(catalog.begin(),catalog.end(),[](const auto& item) { return item.section==Section::drives; })) return E_FAIL;
                }
                return S_OK;
            });
            if(SUCCEEDED(initialized)) CoUninitialize();
        });
    for(auto& worker:workers) worker.join();
    require(SUCCEEDED(concurrent[0]) && SUCCEEDED(concurrent[1]),"concurrent enumeration retains recent snapshots and system drives");
    check_win32(SetEnvironmentVariableW(L"MCOS_RECENT_DIR",(fixture/L"target.txt").c_str()));
    const auto unavailable=enumerate_catalog();
    require(std::any_of(unavailable.begin(),unavailable.end(),[](const auto& item) { return item.section==Section::drives; }) &&
        std::any_of(unavailable.begin(),unavailable.end(),[](const auto& item) { return item.section==Section::files; }),
        "a failed optional history source cannot blank drives or saved favorites");
    check_win32(SetEnvironmentVariableW(L"MCOS_RECENT_DIR",source.c_str()));
    settings.name=L"RAPPORTO"; settings.category=RecentCategory::documents; settings.extension=L"PDF";
    save_recent_settings(settings);
    const auto filtered=enumerate_recents();
    require(filtered.size()==1 && recent_info(filtered[0]).name==L"Rapporto è.pdf",
        "name, category and extension filters combine and preserve Unicode names");
    require(recent_settings().extension==L".pdf","extension filter normalizes a bare uppercase extension");
    settings.name=L""; settings.extension=L""; settings.category=RecentCategory::development; save_recent_settings(settings);
    require(enumerate_recents().size()==1 && recent_info(enumerate_recents()[0]).name==L"site.CSS","development category recognizes source files case-insensitively");
    require(recent_category(L".MD")==RecentCategory::documents && recent_category(L".flac")==RecentCategory::audio &&
        recent_category(L".mkv")==RecentCategory::video && recent_category(L".webp")==RecentCategory::images,
        "document, image, audio and video categories classify expected extensions");
    settings.category=RecentCategory::all; save_recent_settings(settings);
    entries=enumerate_recents();
    std::vector<CatalogItem> pinned{entries[0],entries[1]};
    pin_recents(pinned,true);
    require(recent_info(pinned[0]).pinned && recent_info(pinned[1]).pinned,"bulk pin persists both selections");
    // Age out only isolated fixture inputs. Product code never deletes Windows history.
    std::filesystem::remove(source/(recent_info(pinned[0]).name+L".lnk"));
    std::filesystem::remove(source/(recent_info(pinned[1]).name+L".lnk"));
    require(enumerate_recents().size()==4,"pinned recent references survive removal from the Windows source");
    pin_recents(pinned,false);
    require(enumerate_recents().size()==2,"unpinning aged-out entries removes them from the recent section");
    entries=enumerate_recents();
    add_recents_to_favorites({entries[0]});
    const auto favorite=add_favorite(shell_item(parse_pidl(entries[0].path).get()).Get(),static_cast<Section>(0),recent_info(entries[0]).name);
    require(std::filesystem::path(favorite).stem()==recent_info(entries[0]).name,"promotion retains the document display name and deduplicates its target");

    std::vector<Pidl> ids; std::vector<PCUITEMID_CHILD> selected;
    for(const auto& entry:entries) { ids.push_back(shell::make_item_id(entry)); selected.push_back(ids.back().get()); }
    VARIANT value{}; check(shell->GetDetailsEx(selected[0],&PKEY_DateModified,&value));
    require(value.vt==VT_DATE,"native date column exposes a typed date for sorting"); VariantClear(&value);
    for(const auto& key:{PKEY_ItemNameDisplay,PKEY_ItemName,PKEY_FileName})
    {
        check(shell->GetDetailsEx(selected[0],&key,&value));
        require(value.vt==VT_BSTR && value.bstrVal==recent_info(entries[0]).name,
            "recent name aliases expose the document name, not a cache key");
        VariantClear(&value);
    }
    check(shell->GetDetailsEx(selected[0],&PKEY_ItemNameDisplayWithoutExtension,&value));
    require(value.vt==VT_BSTR && value.bstrVal==std::filesystem::path(recent_info(entries[0]).name).stem().wstring(),
        "small-icon name property preserves the document stem"); VariantClear(&value);
    for(const auto& key:{PKEY_PropList_ContentViewModeForBrowse,PKEY_PropList_ContentViewModeForSearch,
                        PKEY_PropList_TileInfo,PKEY_PropList_ExtendedTileInfo})
    {
        check(shell->GetDetailsEx(selected[0],&key,&value));
        require(value.vt==VT_BSTR && value.bstrVal && std::wstring(value.bstrVal).starts_with(L"prop:"),
            "content and tile layouts expose typed property lists"); VariantClear(&value);
    }
    for(const auto& key:{PKEY_LayoutPattern_ContentViewModeForBrowse,PKEY_LayoutPattern_ContentViewModeForSearch})
    {
        check(shell->GetDetailsEx(selected[0],&key,&value));
        require(value.vt==VT_BSTR && wcscmp(value.bstrVal,LAYOUTPATTERN_CVMFB_DELTA)==0,
            "recent content view uses the documented Shell layout"); VariantClear(&value);
    }
    for(const auto& iid:{IID_IPropertyStore,IID_IPropertyStoreFactory,IID_IPropertyStoreCache})
    {
        void* leaked=nullptr;
        const auto result=shell->BindToObject(selected[0],nullptr,iid,&leaked);
        require(result==E_NOINTERFACE && !leaked,"recent property binding cannot bypass launcher presentation");
    }
    ComPtr<IShellItem2> recent_item;
    check(SHCreateItemWithParent(nullptr,shell,selected[0],IID_PPV_ARGS(&recent_item)));
    PWSTR displayed=nullptr; check(recent_item->GetString(PKEY_ItemNameDisplay,&displayed));
    TaskString owned_name(displayed);
    require(displayed==recent_info(entries[0]).name,"Shell property access retains the recent document name");
    SFGAOF flags=SFGAO_CANDELETE|SFGAO_CANRENAME; check(shell->GetAttributesOf(1,selected.data(),&flags));
    require((flags & SFGAO_CANDELETE) && !(flags & SFGAO_CANRENAME),"recent selection supports hide but not target rename");
    ComPtr<IContextMenu> menu;
    check(shell->GetUIObjectOf(nullptr,static_cast<UINT>(selected.size()),selected.data(),IID_IContextMenu,nullptr,
        reinterpret_cast<void**>(menu.GetAddressOf())));
    const auto pin=menu_command(menu.Get(),L"mcos.recent-pin"); invoke_numeric(menu.Get(),pin);
    require(recent_info(entries[0]).pinned && recent_info(entries[1]).pinned,"native multi-selection menu pins every recent item");
    CMINVOKECOMMANDINFOEX unicode{}; unicode.cbSize=sizeof(unicode); unicode.fMask=CMIC_MASK_UNICODE;
    unicode.lpVerbW=L"mcos.recent-unpin";
    check(menu->InvokeCommand(reinterpret_cast<CMINVOKECOMMANDINFO*>(&unicode)));
    require(!recent_info(entries[0]).pinned && !recent_info(entries[1]).pinned,"Unicode menu invocation handles a null ANSI verb safely");
    const auto remove=menu_command(menu.Get(),L"delete"); invoke_numeric(menu.Get(),remove);
    require(enumerate_recents().empty(),"native multi-selection Delete hides all selected recent items");
    require(contents(targets/L"Rapporto è.pdf")==original_pdf && contents(source/L"Rapporto è.pdf.lnk")==windows_pdf,
        "bulk recent removal preserves original bytes and Windows Recent shortcuts");
    restore_hidden_recents(); require(enumerate_recents().size()==2,"hidden items can be restored without changing Windows history");
    auto restored=enumerate_recents();
    std::filesystem::remove(restored[0].path); // Simulate a Shell host deleting the owned cache .lnk directly.
    require(enumerate_recents().size()==1,"direct native cache deletion is remembered as a hide on refresh");
    restore_hidden_recents(); require(enumerate_recents().size()==2,"directly removed cache item can be restored");
    auto forged=shell::make_item_id({Section::recents,(source/L"Rapporto è.pdf.lnk").wstring()});
    require(guarded([&]{ (void)shell::read_item_id(forged.get()); return S_OK; })==E_ACCESSDENIED,
        "forged recent PIDL cannot mutate a Windows history shortcut");
    settings.enabled=false; save_recent_settings(settings);
    require(enumerate_recents().empty(),"recent section can be turned off independently of favorites");
}

void collection_view_tests(const std::filesystem::path& fixture,IShellFolder2* shell)
{
    const auto original_settings=view_settings();
    const auto first_path=fixture/L"Alpha.PDF",second_path=fixture/L"Beta.docx";
    std::ofstream(first_path)<<"PDF fixture original";
    std::ofstream(second_path)<<"Office fixture original";
    const auto first_link=add_favorite(shell_item(parse_pidl(first_path.wstring()).get()).Get(),Section::files,L"Alias senza suffisso");
    const auto second_link=add_favorite(shell_item(parse_pidl(second_path.wstring()).get()).Get(),Section::files,L"Zeta reference");
    CatalogItem first{Section::files,first_link.wstring()},second{Section::files,second_link.wstring()};
    auto first_id=shell::make_item_id(first),second_id=shell::make_item_id(second);
    PCUITEMID_CHILD selection[]{first_id.get(),second_id.get()};
    auto settings=original_settings;
    settings.families=L"Contratti = .PDF, .docx\r\nSviluppo = cpp; h; md";
    const auto families=parse_families(settings.families);
    require(item_extension(first)==L".pdf","extensions use target suffix, not renamed favorite alias or .lnk");
    require(item_family(first,families)==L"Contratti" && item_family(second,families)==L"Contratti",
        "custom family combines different target extensions, including mixed case");
    require(item_family({Section::folders,first.path},families)==text(IDS_FOLDERS),"folder names are not treated as file extensions");
    settings.field=SearchField::name; settings.query=L"ALIAS";
    require(matches_search(first,settings) && !matches_search(second,settings),"name search is case-insensitive and honors launcher aliases");
    settings.query=L"Alpha";
    require(!matches_search(first,settings),"name-only search does not accidentally search the target path");
    settings.field=SearchField::path;
    require(matches_search(first,settings) && !matches_search(second,settings),"path search uses the target, not the stored shortcut filename");
    settings.field=SearchField::extension; settings.query=L"PDF";
    require(matches_search(first,settings) && !matches_search(second,settings),"extension search normalizes dot and case and matches exactly");
    settings.query=L"pd"; require(!matches_search(first,settings),"extension search does not match a partial suffix");
    settings.field=SearchField::any; settings.query=L".pdf";
    require(matches_search(first,settings),"combined search finds target extensions when the alias omits them");
    settings.query=L""; require(matches_search(first,settings),"clearing search restores a previously filtered reference");
    settings.grouping=GroupMode::families; settings.order=ItemOrder::name; settings.descending=false;
    settings.query=L"Contrattò_日本語";
    save_view_settings(settings);
    const auto loaded=view_settings();
    require(loaded.query==settings.query && loaded.families==settings.families && loaded.grouping==GroupMode::families && !loaded.descending,
        "atomic Unicode settings preserve custom families, grouping and order");
    auto invalid=settings; invalid.families=L"One = pdf\nTwo = .PDF";
    require(guarded([&]{save_view_settings(invalid); return S_OK;})==E_INVALIDARG && view_settings().families==settings.families,
        "duplicate extensions are rejected without replacing the last valid settings");
    for(const auto bad:{L"No equals sign",L"Empty =",L"Unsafe = ../pdf",L"One = pdf\nOne = docx"})
        require(guarded([&]{parse_families(bad); return S_OK;})==E_INVALIDARG,"malformed/ambiguous family definitions are rejected");

    ComPtr<ICategoryProvider> provider; check(shell->CreateViewObject(nullptr,IID_PPV_ARGS(&provider)));
    GUID category{}; check(provider->GetCategoryForSCID(&PKEY_KindText,&category));
    ComPtr<ICategorizer> categorizer; check(provider->CreateCategory(&category,IID_PPV_ARGS(&categorizer)));
    DWORD groups[2]{}; check(categorizer->GetCategory(2,selection,groups));
    CATEGORY_INFO info{}; check(categorizer->GetCategoryInfo(groups[0],&info));
    require(groups[0]==groups[1] && wcscmp(info.wszName,L"Contratti")==0,"Shell family categorizer exposes one real group across extensions");
    settings.families=L"Documenti = pdf, docx"; save_view_settings(settings);
    check(categorizer->GetCategory(2,selection,groups)); check(categorizer->GetCategoryInfo(groups[0],&info));
    require(wcscmp(info.wszName,L"Documenti")==0,"an existing Shell categorizer picks up edited family definitions");
    check(provider->GetCategoryForSCID(&PKEY_FileExtension,&category)); categorizer.Reset();
    check(provider->CreateCategory(&category,IID_PPV_ARGS(&categorizer)));
    check(categorizer->GetCategory(2,selection,groups)); check(categorizer->GetCategoryInfo(groups[0],&info));
    require(groups[0]!=groups[1] && wcscmp(info.wszName,L".pdf")==0,"Shell extension grouping separates actual target suffixes");
    ComPtr<IFolderViewSettings> defaults; check(shell->QueryInterface(IID_PPV_ARGS(&defaults)));
    SORTCOLUMN sort[2]{}; UINT sorts{}; check(defaults->GetSortColumns(sort,2,&sorts));
    require(sorts==1 && IsEqualPropertyKey(sort[0].propkey,PKEY_ItemNameDisplay) && sort[0].direction==SORT_ASCENDING,
        "name ordering is independent from family grouping");
    settings.query=L"pdf"; settings.field=SearchField::extension; save_view_settings(settings);
    ComPtr<IEnumIDList> filtered; check(shell->EnumObjects(nullptr,SHCONTF_FOLDERS|SHCONTF_NONFOLDERS,&filtered));
    PITEMID_CHILD raw{}; ULONG fetched{}; unsigned found{};
    while(filtered->Next(1,&raw,&fetched)==S_OK)
    { Pidl id(raw); require(item_extension(shell::read_item_id(raw))==L".pdf","Shell enumeration filters every section by target extension"); ++found; }
    require(found>0,"filtered enumeration retains matching launcher references");
    settings.query=L"impossible-unique-fixture-search"; save_view_settings(settings);
    filtered.Reset(); check(shell->EnumObjects(nullptr,SHCONTF_FOLDERS|SHCONTF_NONFOLDERS,&filtered));
    require(filtered->Next(1,&raw,&fetched)==S_FALSE && fetched==0,"a no-match search safely returns an empty Shell collection");

    const auto set_date=[](const std::filesystem::path& path,WORD year){
        HANDLE file=CreateFileW(path.c_str(),FILE_WRITE_ATTRIBUTES,FILE_SHARE_READ|FILE_SHARE_WRITE,nullptr,OPEN_EXISTING,0,nullptr);
        check_win32(file!=INVALID_HANDLE_VALUE);
        SYSTEMTIME system{}; system.wYear=year; system.wMonth=1; system.wDay=1; FILETIME time{};
        const BOOL converted=SystemTimeToFileTime(&system,&time); const BOOL saved=converted && SetFileTime(file,nullptr,nullptr,&time);
        const auto error=GetLastError(); CloseHandle(file); if(!saved) check(HRESULT_FROM_WIN32(error));
    };
    set_date(first_path,2020); set_date(second_path,2026);
    VARIANT date{}; check(shell->GetDetailsEx(first_id.get(),&PKEY_ItemDate,&date));
    require(date.vt==VT_DATE,"favorite item date is a typed target modification date"); VariantClear(&date);
    require(static_cast<short>(HRESULT_CODE(shell->CompareIDs(7,first_id.get(),second_id.get())))<0,
        "date comparison is chronological rather than formatted-text ordering");
    settings.query.clear(); settings.order=ItemOrder::date; settings.descending=true; save_view_settings(settings);
    check(defaults->GetSortColumns(sort,2,&sorts));
    require(sorts==2 && IsEqualPropertyKey(sort[0].propkey,PKEY_ItemDate) && sort[0].direction==SORT_DESCENDING &&
        IsEqualPropertyKey(sort[1].propkey,PKEY_ItemNameDisplay),"date ordering has a stable name tie-breaker");
    require(contents(first_path)=="PDF fixture original" && contents(second_path)=="Office fixture original",
        "search, grouping and settings preserve every original file's bytes");
    std::ofstream(data_root()/L"view.ini",std::ios::binary)<<"Interrupted/corrupt preference fixture";
    require(view_settings().query.empty() && view_settings().grouping==GroupMode::sections,
        "a corrupt optional settings file falls back to an unfiltered collection");
    remove_favorite(first); remove_favorite(second); save_view_settings(original_settings);
}
}

int wmain(int count,wchar_t** arguments)
{
    if(count!=3) { std::cerr<<"Usage: NativeTests <absolute DLL> <new fixture directory>\n"; return 2; }
    const auto initialized=CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    if(FAILED(initialized)) return 2;
    HMODULE module=nullptr;
    const auto status=guarded([&]{
        const auto fixture=std::filesystem::absolute(arguments[2]);
        require(!std::filesystem::exists(fixture),"fixture directory is new");
        std::filesystem::create_directories(fixture/L"target-folder");
        std::ofstream(fixture/L"target.txt")<<"Keep the original intact.";
        check_win32(SetEnvironmentVariableW(L"MCOS_DATA_DIR",(fixture/L"store").c_str()));
        const auto folder=add_favorite(shell_item(parse_pidl((fixture/L"target-folder").wstring()).get()).Get());
        const auto file=add_favorite(shell_item(parse_pidl((fixture/L"target.txt").wstring()).get()).Get());
        wchar_t executable[32768]{}; GetModuleFileNameW(nullptr,executable,ARRAYSIZE(executable));
        const auto app=add_favorite(shell_item(parse_pidl(executable).get()).Get());
        require(folder.parent_path().filename()==L"Folders" && file.parent_path().filename()==L"Files" &&
            app.parent_path().filename()==L"Apps","automatic classification places all three favorite kinds");
        const auto duplicate=add_favorite(shell_item(parse_pidl((fixture/L"target.txt").wstring()).get()).Get());
        require(file==duplicate,"adding the same target does not duplicate a favorite");
        const auto original=contents(fixture/L"target.txt");
        write_test_icon(fixture/L"test.ico");
        change_favorite_icon({Section::files,file.wstring()},fixture/L"test.ico");
        {
            ComPtr<IShellLinkW> link; check(CoCreateInstance(CLSID_ShellLink,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&link)));
            ComPtr<IPersistFile> persisted; check(link.As(&persisted)); check(persisted->Load(file.c_str(),STGM_READ));
            wchar_t icon[32768]{}; int index=-1; check(link->GetIconLocation(icon,ARRAYSIZE(icon),&index));
            require(std::filesystem::path(icon)==fixture/L"test.ico" && index==0,"ICO override is saved only on the owned favorite link");
            require(contents(fixture/L"target.txt")==original,"icon override preserves original file bytes");
        }
        std::ofstream(fixture/L"bad.ico")<<"Not an icon";
        require(guarded([&]{ change_favorite_icon({Section::files,file.wstring()},fixture/L"bad.ico"); return S_OK; })==HRESULT_FROM_WIN32(ERROR_INVALID_DATA),
            "invalid ICO is rejected before changing a favorite");
        require(guarded([&]{ change_favorite_icon({Section::drives,L"C:\\"},fixture/L"test.ico"); return S_OK; })==E_ACCESSDENIED,
            "drive icon cannot be changed through the launcher favorite command");

        module=LoadLibraryExW(arguments[1],nullptr,LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR|LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
        require(module!=nullptr,"compiled DLL loads without registration");
        using FactoryFunction=HRESULT(__stdcall*)(REFCLSID,REFIID,void**);
        using UnloadFunction=HRESULT(__stdcall*)();
        auto get_factory=std::bit_cast<FactoryFunction>(GetProcAddress(module,"DllGetClassObject"));
        auto can_unload=std::bit_cast<UnloadFunction>(GetProcAddress(module,"DllCanUnloadNow"));
        require(get_factory && can_unload && can_unload()==S_OK,"COM exports and initial unload state");
        {
            ComPtr<IClassFactory> factory;
            check(get_factory(identity::launcher_clsid,IID_PPV_ARGS(&factory)));
            check(factory->LockServer(TRUE)); require(can_unload()==S_FALSE,"server lock prevents unload");
            check(factory->LockServer(FALSE));
            ComPtr<IShellFolder2> shell;
            check(factory->CreateInstance(nullptr,IID_PPV_ARGS(&shell)));
            ComPtr<IPersistFolder2> persistence; check(shell.As(&persistence));
            ITEMIDLIST desktop{}; check(persistence->Initialize(&desktop));
            PIDLIST_ABSOLUTE raw_root=nullptr; check(persistence->GetCurFolder(&raw_root)); Pidl root(raw_root);
            require(root && root.get()!=&desktop && ILIsEmpty(root.get()),"folder clones root PIDL ownership");
            STRRET canonical{}; check(shell->GetDisplayNameOf(&desktop,SHGDN_FORPARSING,&canonical));
            TaskString canonical_name(canonical.pOleStr);
            require(canonical.uType==STRRET_WSTR && wcscmp(canonical_name.get(),identity::parsing_name)==0,
                "root parsing name retains its absolute Shell namespace identity");

            ComPtr<IEnumIDList> enumerator;
            check(shell->EnumObjects(nullptr,SHCONTF_FOLDERS|SHCONTF_NONFOLDERS,&enumerator));
            std::vector<Pidl> ids; std::array<unsigned,5> sections{};
            PITEMID_CHILD raw=nullptr; ULONG fetched=0;
            while(enumerator->Next(1,&raw,&fetched)==S_OK)
            {
                ids.emplace_back(raw);
                const auto item=mcos::shell::read_item_id(raw);
                ++sections[static_cast<size_t>(item.section)];
                SFGAOF attributes=SFGAO_CANDELETE;
                PCUITEMID_CHILD child=raw; check(shell->GetAttributesOf(1,&child,&attributes));
                require((attributes & SFGAO_CANDELETE)==(item.section==Section::drives ? 0u : SFGAO_CANDELETE),
                    "drive removal is blocked and favorite removal is allowed");
                ComPtr<IContextMenu> menu;
                check(shell->GetUIObjectOf(nullptr,1,&child,IID_IContextMenu,nullptr,reinterpret_cast<void**>(menu.GetAddressOf())));
                HMENU popup=CreatePopupMenu();
                const auto menu_status=menu->QueryContextMenu(popup,0,1,0x7fff,CMF_NORMAL);
                if(SUCCEEDED(menu_status)) verify_launcher_icons(popup,menu.Get());
                DestroyMenu(popup);
                require(SUCCEEDED(menu_status),"native item context menu composes");
            }
            require(sections[1]==1 && sections[2]>0 && sections[3]==1 && sections[4]==1,"all four sections enumerate");
            std::vector<PCUITEMID_CHILD> favorite_selection;
            for(const auto& id:ids)
                if(mcos::shell::read_item_id(id.get()).section!=Section::drives) favorite_selection.push_back(id.get());
            ComPtr<IContextMenu> mixed_menu;
            check(shell->GetUIObjectOf(nullptr,static_cast<UINT>(favorite_selection.size()),favorite_selection.data(),
                IID_IContextMenu,nullptr,reinterpret_cast<void**>(mixed_menu.GetAddressOf())));
            HMENU mixed_popup=CreatePopupMenu();
            const auto mixed_status=mixed_menu->QueryContextMenu(mixed_popup,0,1,0x7fff,CMF_NORMAL);
            DestroyMenu(mixed_popup);
            require(SUCCEEDED(mixed_status),"native mixed-section favorite menu composes");
            ComPtr<IDataObject> mixed_data;
            check(shell->GetUIObjectOf(nullptr,static_cast<UINT>(favorite_selection.size()),favorite_selection.data(),
                IID_IDataObject,nullptr,reinterpret_cast<void**>(mixed_data.GetAddressOf())));
            ComPtr<IShellItemArray> transferred;
            check(SHCreateShellItemArrayFromDataObject(mixed_data.Get(),IID_PPV_ARGS(&transferred)));
            DWORD transferred_count{}; check(transferred->GetCount(&transferred_count));
            require(transferred_count==favorite_selection.size(),"mixed-section Shell data object round-trips every favorite");
            check(enumerator->Reset());
            ComPtr<IEnumIDList> clone; check(enumerator->Clone(&clone));
            PITEMID_CHILD pair[2]{}; check(clone->Next(2,pair,&fetched));
            Pidl first(pair[0]),second(pair[1]);
            require(fetched==2,"enumerator supports clone and batched Next");

            ComPtr<ICategoryProvider> categories;
            check(shell->CreateViewObject(nullptr,IID_PPV_ARGS(&categories)));
            GUID group{}; PROPERTYKEY key{}; check(categories->GetDefaultCategory(&group,&key));
            require(IsEqualPropertyKey(key,PKEY_ItemTypeText),"default grouping uses section property");
            ComPtr<IFolderViewSettings> defaults; check(shell.As(&defaults));
            ComPtr<IPropertyDescriptionList> column_list; check(defaults->GetColumnPropertyList(IID_PPV_ARGS(&column_list)));
            UINT columns{}; check(column_list->GetCount(&columns));
            require(columns==8,"native details view offers extension, family and item-date columns");
            for(UINT index=0;index<columns;++index)
            {
                ComPtr<IPropertyDescription> description; check(column_list->GetAt(index,IID_PPV_ARGS(&description)));
                PROPDESC_VIEW_FLAGS view_flags{}; check(description->GetViewFlags(&view_flags));
                require(((view_flags & PDVF_SHOWBYDEFAULT)!=0)==(index<5),"original columns remain default, organization columns are available");
            }
            ComPtr<ICategorizer> categorizer;
            check(categories->CreateCategory(&group,IID_PPV_ARGS(&categorizer)));
            for(DWORD section=1;section<=5;++section)
            { CATEGORY_INFO info{}; check(categorizer->GetCategoryInfo(section,&info)); require(info.wszName[0]!=0,"category has native display text"); }

            ComPtr<IShellView> view; check(shell->CreateViewObject(nullptr,IID_PPV_ARGS(&view)));
            require(view!=nullptr,"system DefView object is created");
            view.Reset();

            std::ofstream(fixture/L"alias-original.txt")<<"Reference display must not inherit this filename.";
            const auto alias_link=add_favorite(shell_item(parse_pidl((fixture/L"alias-original.txt").wstring()).get()).Get());
            const auto alias=rename_favorite({Section::files,alias_link.wstring()},L"My launcher alias");
            const auto alias_id=shell::make_item_id(alias);
            ComPtr<IShellItem2> alias_item;
            check(SHCreateItemWithParent(nullptr,shell.Get(),alias_id.get(),IID_PPV_ARGS(&alias_item)));
            PWSTR alias_name=nullptr; check(alias_item->GetString(PKEY_ItemNameDisplay,&alias_name));
            TaskString owned_alias(alias_name);
            require(wcscmp(alias_name,L"My launcher alias")==0,"Shell properties preserve a renamed launcher reference");
            PROPVARIANT alias_date{}; check(alias_item->GetProperty(PKEY_DateModified,&alias_date));
            require(alias_date.vt==VT_EMPTY,"favorites do not inherit a target date into the recent-date column");
            PropVariantClear(&alias_date); alias_item.Reset(); remove_favorite(alias);

            std::ofstream(fixture/L"dropped.txt")<<"Drop must preserve this file.";
            auto source=shell_item(parse_pidl((fixture/L"dropped.txt").wstring()).get());
            ComPtr<IDataObject> data; check(source->BindToHandler(nullptr,BHID_DataObject,IID_PPV_ARGS(&data)));
            ComPtr<IDropTarget> drop; check(shell->CreateViewObject(nullptr,IID_PPV_ARGS(&drop)));
            DWORD effect=DROPEFFECT_COPY|DROPEFFECT_MOVE|DROPEFFECT_LINK;
            check(drop->DragEnter(data.Get(),0,{},&effect));
            require(effect==DROPEFFECT_LINK,"drop negotiates a link, never a move");
            check(drop->Drop(data.Get(),0,{},&effect));
            require(effect==DROPEFFECT_LINK && std::filesystem::exists(fixture/L"dropped.txt"),"drop creates a favorite and preserves source");

            std::ofstream(fixture/L"menu-original.txt")<<"Native Delete must preserve this original.";
            const auto reference=add_favorite(shell_item(parse_pidl((fixture/L"menu-original.txt").wstring()).get()).Get());
            auto reference_id=shell::make_item_id({Section::files,reference.wstring()});
            PCUITEMID_CHILD selected=reference_id.get();
            ComPtr<IContextMenu> remove_menu;
            check(shell->GetUIObjectOf(nullptr,1,&selected,IID_IContextMenu,nullptr,
                reinterpret_cast<void**>(remove_menu.GetAddressOf())));
            HMENU remove_popup=CreatePopupMenu();
            check(remove_menu->QueryContextMenu(remove_popup,0,1,0x7fff,CMF_NORMAL));
            UINT delete_command=0;
            for(int index=0;index<GetMenuItemCount(remove_popup);++index)
            {
                const UINT command=GetMenuItemID(remove_popup,index);
                if(command==static_cast<UINT>(-1) || !command) continue;
                wchar_t verb[128]{};
                if(SUCCEEDED(remove_menu->GetCommandString(command-1,GCS_VERBW,nullptr,
                    reinterpret_cast<LPSTR>(verb),ARRAYSIZE(verb))) && wcscmp(verb,L"delete")==0)
                    delete_command=command;
            }
            DestroyMenu(remove_popup);
            require(delete_command!=0,"native favorite menu supplies a canonical Delete command");
            CMINVOKECOMMANDINFO invoke{}; invoke.cbSize=sizeof(invoke);
            invoke.lpVerb=MAKEINTRESOURCEA(delete_command-1); invoke.nShow=SW_HIDE;
            check(remove_menu->InvokeCommand(&invoke));
            require(!std::filesystem::exists(reference) && std::filesystem::exists(fixture/L"menu-original.txt"),
                "native menu Delete removes only the favorite shortcut");
            auto icon_id=shell::make_item_id({Section::files,file.wstring()}); PCUITEMID_CHILD icon_selected=icon_id.get();
            ComPtr<IContextMenu> icon_menu;
            check(shell->GetUIObjectOf(nullptr,1,&icon_selected,IID_IContextMenu,nullptr,reinterpret_cast<void**>(icon_menu.GetAddressOf())));
            require(menu_command(icon_menu.Get(),L"mcos.change-icon")!=0,"native favorite menu exposes Change launcher icon");
            recent_tests(fixture,shell.Get());
            collection_view_tests(fixture,shell.Get());
        }
        require(can_unload()==S_OK,"COM objects release and DLL can unload");

        ComPtr<IShellLinkW> source_link;
        check(CoCreateInstance(CLSID_ShellLink,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&source_link)));
        check(source_link->SetPath(executable)); check(source_link->SetArguments(L"--sample-argument"));
        check(source_link->SetWorkingDirectory(fixture.c_str()));
        ComPtr<IPersistFile> source_file; check(source_link.As(&source_file));
        const auto original_link=fixture/L"original-app.lnk"; check(source_file->Save(original_link.c_str(),TRUE));
        const auto original_link_bytes=contents(original_link);
        const auto copied=add_favorite(shell_item(parse_pidl(original_link.wstring()).get()).Get());
        change_favorite_icon({Section::apps,copied.wstring()},fixture/L"test.ico");
        ComPtr<IShellLinkW> copied_link;
        check(CoCreateInstance(CLSID_ShellLink,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&copied_link)));
        ComPtr<IPersistFile> copied_file; check(copied_link.As(&copied_file)); check(copied_file->Load(copied.c_str(),STGM_READ));
        wchar_t copied_arguments[128]{},working_directory[32768]{};
        check(copied_link->GetArguments(copied_arguments,ARRAYSIZE(copied_arguments)));
        check(copied_link->GetWorkingDirectory(working_directory,ARRAYSIZE(working_directory)));
        require(wcscmp(copied_arguments,L"--sample-argument")==0 && std::filesystem::path(working_directory)==fixture &&
            contents(original_link)==original_link_bytes,"icon override preserves launch arguments, working directory and original shortcut bytes");

        const auto renamed=rename_favorite({Section::files,file.wstring()},L"Renamed reference");
        require(std::filesystem::exists(renamed.path) && std::filesystem::exists(fixture/L"target.txt"),
            "renaming a favorite preserves its original target");
        const auto escaped=guarded([&]{ (void)rename_favorite(renamed,L"..\\outside"); return S_OK; });
        require(escaped==E_INVALIDARG,"favorite rename rejects escaping its store");
        remove_favorite(renamed);
        require(!std::filesystem::exists(renamed.path) && std::filesystem::exists(fixture/L"target.txt"),"favorite removal preserves original file");
        const auto forbidden=guarded([&]{ remove_favorite({Section::drives,L"C:\\"}); return S_OK; });
        require(forbidden==E_ACCESSDENIED,"core policy independently rejects drive removal");
        auto invalid=shell::make_item_id({Section::files,(fixture/L"target.txt").wstring()});
        const auto invalid_status=guarded([&]{ (void)shell::read_item_id(invalid.get()); return S_OK; });
        require(invalid_status==E_ACCESSDENIED,"forged favorite PIDL cannot target an original file");
        return S_OK;
    });
    if(FAILED(status)) std::cerr<<"HRESULT 0x"<<std::hex<<static_cast<unsigned long>(status)<<"\n";
    // Keep the DLL loaded until process teardown if a failed test retained COM state.
    CoUninitialize();
    return FAILED(status) ? 1 : 0;
}
