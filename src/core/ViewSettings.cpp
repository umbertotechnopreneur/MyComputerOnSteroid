// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; atomic native settings and collection filters; 2026-10-11.
 * Human guidance: Umberto Giacobbi; customize extension families, preserve targets.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */
#include "ViewSettings.hpp"
#include "Recents.hpp"
#include "../../resources/resource.h"
#include <algorithm>
#include <array>
#include <fstream>
#include <limits>
#include <set>
#include <sstream>

namespace mcos
{
namespace
{
std::wstring trim(const std::wstring& value)
{
    const auto first=value.find_first_not_of(L" \t\r\n");
    return first==std::wstring::npos ? L"" : value.substr(first,value.find_last_not_of(L" \t\r\n")-first+1);
}
std::wstring lower(std::wstring value)
{
    std::transform(value.begin(),value.end(),value.begin(),[](wchar_t c){return static_cast<wchar_t>(towlower(c));});
    return value;
}
void validate_path(const std::filesystem::path& path, bool directory)
{
    const auto flags=GetFileAttributesW(path.c_str());
    if(flags==INVALID_FILE_ATTRIBUTES)
    {
        const auto error=GetLastError();
        if(error!=ERROR_FILE_NOT_FOUND && error!=ERROR_PATH_NOT_FOUND) check(HRESULT_FROM_WIN32(error));
        return;
    }
    if((flags & FILE_ATTRIBUTE_REPARSE_POINT) || ((flags & FILE_ATTRIBUTE_DIRECTORY)!=0)!=directory) check(E_ACCESSDENIED);
}
ViewSettings defaults()
{ ViewSettings settings; settings.families=text(IDS_FAMILY_DEFAULTS); return settings; }
void validate(const ViewSettings& settings)
{
    if(static_cast<unsigned>(settings.grouping)>3 || static_cast<unsigned>(settings.order)>1 ||
       static_cast<unsigned>(settings.field)>3 || settings.query.size()>256 || settings.families.size()>16384 ||
       settings.query.find_first_of(L"\r\n\0",0,3)!=std::wstring::npos) check(E_INVALIDARG);
    parse_families(settings.families);
}
}

std::vector<ExtensionFamily> parse_families(const std::wstring& definitions)
{
    if(definitions.size()>16384 || definitions.find(L'\0')!=std::wstring::npos) check(E_INVALIDARG);
    std::vector<ExtensionFamily> families;
    std::set<std::wstring> extensions,names;
    std::wistringstream lines(definitions);
    std::wstring line;
    while(std::getline(lines,line))
    {
        line=trim(line); if(line.empty()) continue;
        const auto equals=line.find(L'=');
        if(equals==std::wstring::npos || line.find(L'=',equals+1)!=std::wstring::npos) check(E_INVALIDARG);
        ExtensionFamily family{trim(line.substr(0,equals)),{}};
        if(family.name.empty() || family.name.size()>60 || !names.insert(lower(family.name)).second || families.size()>=32)
            check(E_INVALIDARG);
        auto list=line.substr(equals+1);
        std::replace(list.begin(),list.end(),L',',L' '); std::replace(list.begin(),list.end(),L';',L' ');
        std::wistringstream tokens(list); std::wstring token;
        while(tokens>>token)
        {
            if(token.front()==L'.') token.erase(0,1);
            token=lower(token);
            if(token.empty() || token.size()>32 || !std::all_of(token.begin(),token.end(),[](wchar_t c){
                return iswalnum(c) || c==L'_' || c==L'-' || c==L'+'; }) || !extensions.insert(token).second)
                check(E_INVALIDARG);
            family.extensions.push_back(L"."+token);
        }
        if(family.extensions.empty()) check(E_INVALIDARG);
        families.push_back(std::move(family));
    }
    return families;
}

ViewSettings view_settings()
{
    auto settings=defaults();
    // A partial/corrupt optional preference file must not hide the catalog.
    guarded([&]{
        const auto root=data_root(),file=root/L"view.ini";
        validate_path(root,true); validate_path(file,false);
        std::ifstream input(file,std::ios::binary);
        if(!input) return S_OK;
        input.seekg(0,std::ios::end); const auto bytes=input.tellg();
        if(bytes<2 || bytes>65536 || bytes%2!=0) return E_INVALIDARG;
        input.seekg(0); std::wstring content(static_cast<size_t>(bytes)/2,L'\0');
        if(!input.read(reinterpret_cast<char*>(content.data()),bytes) || content.front()!=0xfeff) return E_INVALIDARG;
        std::wistringstream lines(content.substr(1));
        ViewSettings loaded;
        unsigned group{},order{},direction{},field{};
        if(!(lines>>group>>order>>direction>>field) || group>3 || order>1 || direction>1 || field>3) return E_INVALIDARG;
        lines.ignore((std::numeric_limits<std::streamsize>::max)(),L'\n');
        if(!std::getline(lines,loaded.query)) return E_INVALIDARG;
        loaded.families.assign(std::istreambuf_iterator<wchar_t>(lines),{});
        loaded.grouping=static_cast<GroupMode>(group); loaded.order=static_cast<ItemOrder>(order);
        loaded.descending=direction!=0; loaded.field=static_cast<SearchField>(field);
        validate(loaded); settings=std::move(loaded); return S_OK;
    });
    return settings;
}

void save_view_settings(const ViewSettings& settings)
{
    validate(settings);
    const auto root=data_root(),file=root/L"view.ini";
    validate_path(root,true); validate_path(file,false);
    std::filesystem::create_directories(root); validate_path(root,true);
    GUID id{}; check(CoCreateGuid(&id)); wchar_t key[40]{}; StringFromGUID2(id,key,ARRAYSIZE(key));
    const auto temporary=root/(std::wstring(L".mcos-view-")+key+L".tmp");
    // Unique, CREATE_NEW temporary and atomic replacement: cancellation/errors
    // preserve the last settings. Concurrent Apply uses the last complete writer.
    const auto contents=std::wstring(1,0xfeff)+std::to_wstring(static_cast<unsigned>(settings.grouping))+L" "+
        std::to_wstring(static_cast<unsigned>(settings.order))+L" "+std::to_wstring(settings.descending ? 1 : 0)+L" "+
        std::to_wstring(static_cast<unsigned>(settings.field))+L"\n"+trim(settings.query)+L"\n"+settings.families;
    HANDLE handle=CreateFileW(temporary.c_str(),GENERIC_WRITE,0,nullptr,CREATE_NEW,FILE_ATTRIBUTE_NORMAL,nullptr);
    if(handle==INVALID_HANDLE_VALUE) check(HRESULT_FROM_WIN32(GetLastError()));
    DWORD written{}; const auto bytes=static_cast<DWORD>(contents.size()*sizeof(wchar_t));
    const BOOL saved=WriteFile(handle,contents.data(),bytes,&written,nullptr);
    auto error=saved && written==bytes && FlushFileBuffers(handle) ? ERROR_SUCCESS : GetLastError();
    if(!saved || written!=bytes) { if(error==ERROR_SUCCESS) error=ERROR_WRITE_FAULT; }
    CloseHandle(handle);
    if(error==ERROR_SUCCESS)
    {
        const auto status=guarded([&]{validate_path(root,true); validate_path(file,false); return S_OK;});
        if(FAILED(status)) { DeleteFileW(temporary.c_str()); check(status); }
        if(!MoveFileExW(temporary.c_str(),file.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)) error=GetLastError();
    }
    if(error!=ERROR_SUCCESS) { DeleteFileW(temporary.c_str()); check(HRESULT_FROM_WIN32(error)); }
    notify_launcher();
}

std::wstring item_target_path(const CatalogItem& item)
{
    if(item.section==Section::drives) return item.path;
    if(!(item.section==Section::recents ? is_owned_recent(item) : is_owned_favorite(item))) check(E_ACCESSDENIED);
    ComPtr<IShellLinkW> link; check(CoCreateInstance(CLSID_ShellLink,nullptr,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&link)));
    ComPtr<IPersistFile> file; check(link.As(&file)); check(file->Load(item.path.c_str(),STGM_READ));
    std::array<wchar_t,32768> path{};
    check(link->GetPath(path.data(),static_cast<int>(path.size()),nullptr,SLGP_RAWPATH));
    return path.data(); // No Resolve, filesystem scan, virtual binding or execution.
}
std::wstring item_extension(const CatalogItem& item)
{
    if(item.section==Section::drives || item.section==Section::folders) return {};
    return lower(std::filesystem::path(item_target_path(item)).extension().wstring());
}
std::wstring item_family(const CatalogItem& item,const std::vector<ExtensionFamily>& families)
{
    if(item.section==Section::drives) return text(IDS_DRIVES);
    if(item.section==Section::folders) return text(IDS_FOLDERS);
    const auto extension=item_extension(item);
    for(const auto& family:families)
        if(std::find(family.extensions.begin(),family.extensions.end(),extension)!=family.extensions.end()) return family.name;
    return text(extension.empty() ? IDS_NO_EXTENSION : IDS_OTHER_EXTENSIONS);
}
FILETIME item_date(const CatalogItem& item)
{
    if(item.section==Section::recents) return recent_info(item).updated;
    if(item.section==Section::drives) return {};
    const auto path=item_target_path(item);
    // Network/offline targets must not stall Explorer for optional date metadata.
    if(path.empty() || PathIsNetworkPathW(path.c_str()) || GetDriveTypeW(std::filesystem::path(path).root_path().c_str())==DRIVE_REMOTE)
        return {};
    WIN32_FILE_ATTRIBUTE_DATA data{};
    return GetFileAttributesExW(path.c_str(),GetFileExInfoStandard,&data) ? data.ftLastWriteTime : FILETIME{};
}
bool matches_search(const CatalogItem& item,const ViewSettings& settings)
{
    auto query=trim(settings.query); if(query.empty()) return true;
    const auto contains=[&](const std::wstring& value){return StrStrIW(value.c_str(),query.c_str())!=nullptr;};
    if(settings.field==SearchField::extension)
    {
        if(query.front()!=L'.') query=L"."+query;
        return _wcsicmp(item_extension(item).c_str(),query.c_str())==0;
    }
    if((settings.field==SearchField::any || settings.field==SearchField::name) && contains(item_name(item))) return true;
    return (settings.field==SearchField::any || settings.field==SearchField::path) && contains(item_target_path(item));
}
}
