// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; native launcher implementation; 2026-10-11.
 * Modified with AI: OpenAI Codex; explicit menu actions and readable drop flow; 2026-10-17.
 * Human guidance: Umberto Giacobbi; standard Shell view, native menus and MSI.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#include "ShellFolder.hpp"
#include "Module.hpp"
#include "ItemId.hpp"
#include "Grouping.hpp"
#include "FavoriteMenu.hpp"
#include "../core/Recents.hpp"
#include "../core/ViewSettings.hpp"
#include "../native/Dialog.hpp"
#include "../ProjectIdentity.hpp"
#include "../../resources/resource.h"
#include <Propkey.h>
#include <Propvarutil.h>
#include <Propsys.h>
#include <CommCtrl.h>
#include <algorithm>
#include <array>

namespace mcos::shell
{
namespace
{
enum class BackgroundAction { configure };

struct BackgroundCommand
{
    BackgroundAction action;
    UINT label;
    MenuGlyph glyph;
    UINT error_label;
};

// Settings belong to the configuration window, not a growing background menu.
constexpr std::array BackgroundCommands{
    BackgroundCommand{BackgroundAction::configure, IDS_CONFIGURE, MenuGlyph::settings, IDS_ERROR_CONFIGURATION}
};

const BackgroundCommand* background_command(WPARAM offset) noexcept
{
    return offset < BackgroundCommands.size() ? &BackgroundCommands[offset] : nullptr;
}

std::filesystem::path launcher_path()
{
    return module_path(current_module()).parent_path() / L"MyComputerOnSteroid.exe";
}


HRESULT add_dropped_folders_to_this_pc(HWND owner, IShellItemArray* items)
{
    DWORD count{};
    check(items->GetCount(&count));
    if (!count) return S_FALSE;

    std::wstring parameters{L"--add-this-pc"};
    for (DWORD index = 0; index < count; ++index)
    {
        ComPtr<IShellItem> item;
        check(items->GetItemAt(index, &item));
        constexpr SFGAOF required = SFGAO_FOLDER | SFGAO_FILESYSTEM;
        SFGAOF attributes{};
        check(item->GetAttributes(required, &attributes));
        const bool is_real_folder = (attributes & required) == required;
        if (!is_real_folder) return S_FALSE;

        const auto path = display_name(item.Get(), SIGDN_FILESYSPATH);
        parameters += L" ";
        parameters += quote_command_line_argument(path);
    }

    // S_FALSE above means "use favorites". Cancellation after choosing the
    // direct-folder route means "make no change", never "use favorites".
    const auto status = launch_process(owner, launcher_path(), parameters);
    return status == S_FALSE ? S_OK : status;
}

// Enumerates a snapshot: PIDLs returned to Explorer are independent allocations.
class ItemEnumerator final : public IEnumIDList
{
public:
    explicit ItemEnumerator(std::vector<CatalogItem> items,size_t index=0)
        : items_(std::move(items)),index_(index) { ++live_objects; }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** result) noexcept override
    {
        if(!result) return E_POINTER;
        *result=nullptr;
        if(iid!=IID_IUnknown && iid!=IID_IEnumIDList) return E_NOINTERFACE;
        *result=static_cast<IEnumIDList*>(this); AddRef(); return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() noexcept override { return ++references_; }
    ULONG STDMETHODCALLTYPE Release() noexcept override
    { auto left=--references_; if(!left) delete this; return left; }
    HRESULT STDMETHODCALLTYPE Next(ULONG count,PITEMID_CHILD* items,ULONG* fetched) noexcept override
    {
        if(fetched) *fetched=0;
        if(!count) return S_OK;
        if(!items || (!fetched && count!=1)) return E_POINTER;
        std::fill_n(items,count,nullptr);
        ULONG returned=0;
        const auto status=guarded([&]{
            for(;returned<count && index_+returned<items_.size();++returned)
                items[returned]=make_item_id(items_[index_+returned]).release();
            return returned==count ? S_OK : S_FALSE;
        });
        if(FAILED(status))
        {
            for(ULONG i=0;i<returned;++i) { CoTaskMemFree(items[i]); items[i]=nullptr; }
            return status;
        }
        index_+=returned;
        if(fetched) *fetched=returned;
        return status;
    }
    HRESULT STDMETHODCALLTYPE Skip(ULONG count) noexcept override
    {
        const auto skipped=(std::min)(static_cast<size_t>(count),items_.size()-index_);
        index_+=skipped; return skipped==count ? S_OK : S_FALSE;
    }
    HRESULT STDMETHODCALLTYPE Reset() noexcept override { index_=0; return S_OK; }
    HRESULT STDMETHODCALLTYPE Clone(IEnumIDList** result) noexcept override
    {
        if(!result) return E_POINTER;
        *result=nullptr;
        return guarded([&]{ *result=new ItemEnumerator(items_,index_); return S_OK; });
    }
private:
    ~ItemEnumerator() { --live_objects; }
    std::atomic<ULONG> references_{1};
    std::vector<CatalogItem> items_;
    size_t index_;
};

HRESULT make_string(const std::wstring& source,STRRET* result)
{
    if(!result) return E_POINTER;
    *result={}; result->uType=STRRET_WSTR;
    return SHStrDupW(source.c_str(),&result->pOleStr);
}
FILETIME recent_time(const CatalogItem& item)
{ return item.section==Section::recents ? recent_info(item).updated : FILETIME{}; }
std::wstring formatted_time(FILETIME time)
{
    if(!time.dwLowDateTime && !time.dwHighDateTime) return {};
    FILETIME local{}; SYSTEMTIME system{};
    check_win32(FileTimeToLocalFileTime(&time,&local)); check_win32(FileTimeToSystemTime(&local,&system));
    wchar_t date[128]{},clock[128]{};
    if(!GetDateFormatEx(LOCALE_NAME_USER_DEFAULT,DATE_SHORTDATE,&system,nullptr,date,ARRAYSIZE(date),nullptr) ||
       !GetTimeFormatEx(LOCALE_NAME_USER_DEFAULT,TIME_NOSECONDS,&system,nullptr,clock,ARRAYSIZE(clock)))
        check(HRESULT_FROM_WIN32(GetLastError()));
    return std::wstring(date)+L" "+clock;
}
PROPERTYKEY group_key(GroupMode mode) noexcept
{
    switch (mode)
    {
    case GroupMode::extension: return PKEY_FileExtension;
    case GroupMode::families: return PKEY_KindText;
    case GroupMode::none: return PKEY_Null;
    default: return PKEY_ItemTypeText;
    }
}
std::array<SORTCOLUMN, 2> sort_columns(const ViewSettings& settings)
{
    const auto primary_key = settings.order == ItemOrder::name ? PKEY_ItemNameDisplay : PKEY_ItemDate;
    const auto direction = settings.descending ? SORT_DESCENDING : SORT_ASCENDING;
    return {SORTCOLUMN{primary_key, direction}, SORTCOLUMN{PKEY_ItemNameDisplay, SORT_ASCENDING}};
}

HRESULT invoke_background_command(HWND owner, PCIDLIST_ABSOLUTE, BackgroundAction)
{
    return launch_process(owner, launcher_path(),
        L"--configure" + dialog_origin_arguments(dialog_origin(owner)));
}
std::wstring detail(const CatalogItem& item,UINT column)
{
    switch(column)
    {
    case 0: return item_name(item);
    case 1: return text(section_label(item.section));
    case 2:
        if(item.section==Section::drives) return item.path;
        return display_name(shell_item(navigation_target(item).get()).Get(),SIGDN_DESKTOPABSOLUTEPARSING);
    case 3: return formatted_time(recent_time(item));
    case 4: return item.section==Section::recents && recent_info(item).pinned ? text(IDS_PINNED_YES) : L"";
    case 5: return item_extension(item);
    case 6: return item_family(item,parse_families(view_settings().families));
    case 7: return formatted_time(item_date(item));
    default: check(E_INVALIDARG); return {};
    }
}
DWORD drop_effect(DWORD offered) noexcept
{
    if(offered & DROPEFFECT_LINK) return DROPEFFECT_LINK;
    return offered & DROPEFFECT_COPY; // Never negotiate a move of original data.
}
}
ShellFolder::ShellFolder() noexcept { ++live_objects; }
ShellFolder::~ShellFolder() { --live_objects; }
HRESULT ShellFolder::QueryInterface(REFIID iid,void** result) noexcept
{
    if(!result) return E_POINTER;
    *result=nullptr;
    if(iid==IID_IUnknown || iid==IID_IShellFolder || iid==IID_IShellFolder2) *result=static_cast<IShellFolder2*>(this);
    else if(iid==IID_IPersist || iid==IID_IPersistFolder || iid==IID_IPersistFolder2) *result=static_cast<IPersistFolder2*>(this);
    else if(iid==IID_IShellFolderViewCB) *result=static_cast<IShellFolderViewCB*>(this);
    else if(iid==IID_IContextMenuCB) *result=static_cast<IContextMenuCB*>(this);
    else if(iid==IID_IDropTarget) *result=static_cast<IDropTarget*>(this);
    else if(iid==IID_IFolderViewSettings) *result=static_cast<IFolderViewSettings*>(this);
    else return E_NOINTERFACE;
    AddRef(); return S_OK;
}
ULONG ShellFolder::AddRef() noexcept { return ++references_; }
ULONG ShellFolder::Release() noexcept { auto left=--references_; if(!left) delete this; return left; }
HRESULT ShellFolder::GetClassID(CLSID* id) noexcept
{ if(!id) return E_POINTER; *id=identity::launcher_clsid; return S_OK; }
HRESULT ShellFolder::Initialize(PCIDLIST_ABSOLUTE pidl) noexcept
{
    if(!pidl) return E_INVALIDARG;
    Pidl copy(ILCloneFull(pidl)); if(!copy) return E_OUTOFMEMORY;
    root_=std::move(copy); return S_OK;
}
HRESULT ShellFolder::GetCurFolder(PIDLIST_ABSOLUTE* pidl) noexcept
{
    if(!pidl) return E_POINTER;
    *pidl=nullptr;
    if(!root_) return E_UNEXPECTED;
    *pidl=ILCloneFull(root_.get()); return *pidl ? S_OK : E_OUTOFMEMORY;
}
HRESULT ShellFolder::ParseDisplayName(HWND,IBindCtx*,PWSTR name,ULONG* eaten,PIDLIST_RELATIVE* pidl,ULONG* attributes) noexcept
{
    if(eaten) *eaten=0;
    if(!pidl) return E_POINTER;
    *pidl=nullptr;
    return guarded([&]{
        if(!name) return E_INVALIDARG;
        for(const auto& item:enumerate_catalog())
        {
            if(_wcsicmp(item.path.c_str(),name)!=0) continue;
            *pidl=make_item_id(item).release();
            if(eaten) *eaten=static_cast<ULONG>(wcslen(name));
            if(attributes) { PCUITEMID_CHILD child=*pidl; GetAttributesOf(1,&child,attributes); }
            return S_OK;
        }
        return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
    });
}
HRESULT ShellFolder::EnumObjects(HWND,SHCONTF flags,IEnumIDList** result) noexcept
{
    if(!result) return E_POINTER;
    *result=nullptr;
    auto status=guarded([&]{
        auto items=enumerate_catalog();
        const auto settings=view_settings();
        std::erase_if(items,[&](const CatalogItem& item){
            const bool folder=item.section==Section::folders || item.section==Section::drives;
            if(folder ? !(flags & SHCONTF_FOLDERS) : !(flags & SHCONTF_NONFOLDERS)) return true;
            bool matches=false;
            // A broken reference's optional metadata cannot blank other results.
            guarded([&]{matches=matches_search(item,settings); return S_OK;});
            return !matches;
        });
        *result=new ItemEnumerator(std::move(items)); return S_OK;
    });
    if(FAILED(status)) report_error(nullptr,status,IDS_ERROR_READ);
    return status;
}
HRESULT ShellFolder::BindToObject(PCUIDLIST_RELATIVE pidl,IBindCtx* context,REFIID iid,void** result) noexcept
{
    if(!result) return E_POINTER;
    *result=nullptr;
    return guarded([&]{
        const auto item=read_item_id(pidl);
        // DefView must build launcher properties from this folder. A target's
        // factory bypasses reference names, history dates and layout overrides;
        // Windows history links can also carry source-specific property caches.
        if(iid==IID_IPropertyStore || iid==IID_IPropertyStoreFactory || iid==IID_IPropertyStoreCache)
            return E_NOINTERFACE;
        auto target=navigation_target(item);
        auto tail=ILNext(pidl);
        if(!ILIsEmpty(tail))
        {
            Pidl combined(ILCombine(target.get(),tail)); if(!combined) return E_OUTOFMEMORY;
            target=std::move(combined);
        }
        return SHBindToObject(nullptr,target.get(),context,iid,result);
    });
}
HRESULT ShellFolder::BindToStorage(PCUIDLIST_RELATIVE pidl,IBindCtx* context,REFIID iid,void** result) noexcept
{ return BindToObject(pidl,context,iid,result); }
HRESULT ShellFolder::CompareIDs(LPARAM column,PCUIDLIST_RELATIVE first,PCUIDLIST_RELATIVE second) noexcept
{
    return guarded([&]{
        const auto index=static_cast<UINT>(column & SHCIDS_COLUMNMASK);
        const auto a=read_item_id(first),b=read_item_id(second);
        int comparison=0;
        if(index==3 || index==7)
        {
            const auto first_time=index==3 ? recent_time(a) : item_date(a),second_time=index==3 ? recent_time(b) : item_date(b);
            comparison=CompareFileTime(&first_time,&second_time);
        }
        else comparison=StrCmpLogicalW(detail(a,index).c_str(),detail(b,index).c_str());
        const short order=comparison<0 ? -1 : comparison>0 ? 1 : 0;
        return MAKE_HRESULT(SEVERITY_SUCCESS,0,static_cast<USHORT>(order));
    });
}
HRESULT ShellFolder::CreateViewObject(HWND owner,REFIID iid,void** result) noexcept
{
    if(!result) return E_POINTER;
    *result=nullptr;
    return guarded([&]{
        if(iid==IID_IShellView)
        {
            SFV_CREATE create{}; create.cbSize=sizeof(create); create.pshf=this; create.psfvcb=this;
            ComPtr<IShellView> view; check(SHCreateShellFolderView(&create,&view));
            *result=view.Detach(); return S_OK;
        }
        if(iid==IID_ICategoryProvider) return create_categories(iid,result);
        if(iid==IID_IDropTarget) return QueryInterface(iid,result);
        if(iid==IID_IContextMenu)
        {
            DEFCONTEXTMENU menu{};
            menu.hwnd=owner; menu.pcmcb=this; menu.pidlFolder=root_.get(); menu.psf=this;
            return SHCreateDefaultContextMenu(&menu,iid,result);
        }
        return E_NOINTERFACE;
    });
}
HRESULT ShellFolder::GetAttributesOf(UINT count,PCUITEMID_CHILD_ARRAY items,SFGAOF* attributes) noexcept
{
    if(!attributes) return E_POINTER;
    return guarded([&]{
        if(!count) { *attributes &= SFGAO_FOLDER|SFGAO_BROWSABLE|SFGAO_DROPTARGET|SFGAO_HASSUBFOLDER|SFGAO_CANRENAME; return S_OK; }
        if(!items) return E_POINTER;
        SFGAOF common=*attributes;
        for(UINT i=0;i<count;++i)
        {
            const auto item=read_item_id(items[i]);
            auto native=shell_item(parse_pidl(item.path).get());
            SFGAOF flags=0; check(native->GetAttributes(*attributes,&flags));
            if(item.section==Section::drives) flags &= ~(SFGAO_CANDELETE|SFGAO_CANMOVE|SFGAO_CANRENAME);
            else
            {
                flags |= SFGAO_CANDELETE|SFGAO_LINK|SFGAO_FILESYSTEM;
                if(item.section==Section::recents) flags &= ~SFGAO_CANRENAME;
                else flags |= SFGAO_CANRENAME;
            }
            // Exported document paths are offered for copying, never moving
            // originals out of the launcher. Native mutations still use links.
            flags &= ~(SFGAO_DROPTARGET|SFGAO_CANMOVE);
            if(item.section==Section::folders || item.section==Section::drives)
                flags |= SFGAO_FOLDER|SFGAO_BROWSABLE|SFGAO_HASSUBFOLDER;
            else flags &= ~(SFGAO_FOLDER|SFGAO_HASSUBFOLDER);
            common &= flags;
        }
        *attributes=common; return S_OK;
    });
}
HRESULT ShellFolder::GetUIObjectOf(HWND owner,UINT count,PCUITEMID_CHILD_ARRAY items,REFIID iid,UINT*,void** result) noexcept
{
    if(!result) return E_POINTER;
    *result=nullptr;
    return guarded([&]{
        if(!count || !items) return E_INVALIDARG;
        if(iid==IID_IDataObject)
            return transfer_data_object(count,items,reinterpret_cast<IDataObject**>(result));
        if(iid==IID_IContextMenu || iid==IID_IContextMenu2 || iid==IID_IContextMenu3)
        {
            std::vector<CatalogItem> favorites;
            for(UINT index=0;index<count;++index)
            {
                auto item=read_item_id(items[index]);
                if(item.section==Section::drives) { favorites.clear(); break; }
                favorites.push_back(std::move(item));
            }
            if(!favorites.empty())
            {
                ComPtr<IContextMenu> menu;
                auto status=native_ui_object(owner,count,items,IID_IContextMenu,reinterpret_cast<void**>(menu.GetAddressOf()));
                if(status==HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED))
                {
                    std::vector<Pidl> owned; std::vector<PCUITEMID_CHILD> absolute;
                    for(const auto& item:favorites)
                    { owned.push_back(parse_pidl(item.path)); absolute.push_back(owned.back().get()); }
                    ComPtr<IShellFolder> desktop; check(SHGetDesktopFolder(&desktop));
                    status=desktop->GetUIObjectOf(owner,count,absolute.data(),IID_IContextMenu,nullptr,
                        reinterpret_cast<void**>(menu.GetAddressOf()));
                }
                check(status);
                return favorite_menu(menu.Get(),std::move(favorites),owner,iid,result);
            }
        }
        const auto native=native_ui_object(owner,count,items,iid,result);
        if(SUCCEEDED(native) || native!=HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED)) return native;
        std::vector<Pidl> owned; std::vector<PCIDLIST_ABSOLUTE> absolute;
        for(UINT i=0;i<count;++i)
        {
            owned.push_back(parse_pidl(read_item_id(items[i]).path));
            absolute.push_back(owned.back().get());
        }
        ComPtr<IShellFolder> desktop; check(SHGetDesktopFolder(&desktop));
        return desktop->GetUIObjectOf(owner,count,absolute.data(),iid,nullptr,result);
    });
}
HRESULT ShellFolder::GetDisplayNameOf(PCUITEMID_CHILD pidl,SHGDNF flags,STRRET* result) noexcept
{
    if(!result) return E_POINTER;
    *result={};
    return guarded([&]{
        if(!pidl || ILIsEmpty(pidl))
            return make_string(flags & SHGDN_FORPARSING ? identity::parsing_name : identity::launcher_name,result);
        const auto item=read_item_id(pidl);
        return make_string(flags & SHGDN_FORPARSING ? item.path : item_name(item),result);
    });
}
HRESULT ShellFolder::SetNameOf(HWND,PCUITEMID_CHILD pidl,PCWSTR name,SHGDNF,PITEMID_CHILD* result) noexcept
{
    if(result) *result=nullptr;
    return guarded([&]{
        if(!name) return E_POINTER;
        const auto old=read_item_id(pidl);
        const auto item=rename_favorite(old,name);
        auto renamed=make_item_id(item);
        if(root_)
        {
            Pidl before(ILCombine(root_.get(),pidl)),after(ILCombine(root_.get(),renamed.get()));
            if(before && after)
                SHChangeNotify(old.section==Section::folders ? SHCNE_RENAMEFOLDER : SHCNE_RENAMEITEM,
                    SHCNF_IDLIST|SHCNF_FLUSHNOWAIT,before.get(),after.get());
        }
        if(result) *result=renamed.release();
        return S_OK;
    });
}
HRESULT ShellFolder::GetDefaultSearchGUID(GUID* value) noexcept
{ if(!value) return E_POINTER; *value=GUID_NULL; return E_NOTIMPL; }
HRESULT ShellFolder::EnumSearches(IEnumExtraSearch** result) noexcept
{ if(!result) return E_POINTER; *result=nullptr; return E_NOTIMPL; }
HRESULT ShellFolder::GetDefaultColumn(DWORD,ULONG* sort,ULONG* display) noexcept
{ if(!sort || !display) return E_POINTER;
  return guarded([&]{*sort=view_settings().order==ItemOrder::name ? 0u : 7u; *display=0; return S_OK;}); }
HRESULT ShellFolder::GetDefaultColumnState(UINT column,SHCOLSTATEF* state) noexcept
{
    if(!state) return E_POINTER;
    *state=0;
    if(column>7) return E_INVALIDARG;
    *state=(column==3 || column==7 ? SHCOLSTATE_TYPE_DATE : SHCOLSTATE_TYPE_STR)|
        (column<5 ? SHCOLSTATE_ONBYDEFAULT : 0); return S_OK;
}
HRESULT ShellFolder::GetDetailsEx(PCUITEMID_CHILD pidl,const SHCOLUMNID* key,VARIANT* value) noexcept
{
    if(!value) return E_POINTER;
    VariantInit(value);
    return guarded([&]{
        if(!key) return E_POINTER;
        const auto item=read_item_id(pidl);
        if(IsEqualPropertyKey(*key,PKEY_ItemNameDisplay) || IsEqualPropertyKey(*key,PKEY_ItemName) ||
           IsEqualPropertyKey(*key,PKEY_FileName))
            return InitVariantFromString(item_name(item).c_str(),value);
        if(IsEqualPropertyKey(*key,PKEY_ItemNameDisplayWithoutExtension))
        {
            auto name=item_name(item);
            if(item.section!=Section::drives && item.section!=Section::folders)
                name=std::filesystem::path(name).stem().wstring();
            return InitVariantFromString(name.c_str(),value);
        }
        if(IsEqualPropertyKey(*key,PKEY_ParsingPath))
            return InitVariantFromString(item.path.c_str(),value);
        // DefView asks for association/layout properties as well as visible columns.
        // Reference layouts must use the launcher label, including a renamed link.
        if(item.section!=Section::drives &&
           (IsEqualPropertyKey(*key,PKEY_PropList_ContentViewModeForBrowse) ||
            IsEqualPropertyKey(*key,PKEY_PropList_ContentViewModeForSearch)))
            return InitVariantFromString(item.section==Section::recents ?
                L"prop:~System.ItemNameDisplay;System.KindText;~System.LayoutPattern.PlaceHolder;~System.LayoutPattern.PlaceHolder;System.DateModified;System.Comment" :
                L"prop:~System.ItemNameDisplay;System.KindText;~System.LayoutPattern.PlaceHolder;~System.LayoutPattern.PlaceHolder;System.DateModified;System.Size",value);
        if(item.section!=Section::drives &&
           (IsEqualPropertyKey(*key,PKEY_PropList_TileInfo) || IsEqualPropertyKey(*key,PKEY_PropList_ExtendedTileInfo)))
            return InitVariantFromString(item.section==Section::recents ?
                L"prop:System.KindText;System.DateModified;System.Comment" : L"prop:System.KindText;System.Size",value);
        if(item.section!=Section::drives &&
           (IsEqualPropertyKey(*key,PKEY_LayoutPattern_ContentViewModeForBrowse) ||
            IsEqualPropertyKey(*key,PKEY_LayoutPattern_ContentViewModeForSearch)))
            return InitVariantFromString(LAYOUTPATTERN_CVMFB_DELTA,value);
        if(IsEqualPropertyKey(*key,PKEY_FileExtension)) return InitVariantFromString(detail(item,5).c_str(),value);
        if(IsEqualPropertyKey(*key,PKEY_KindText)) return InitVariantFromString(detail(item,6).c_str(),value);
        if(IsEqualPropertyKey(*key,PKEY_DateModified) || IsEqualPropertyKey(*key,PKEY_ItemDate))
        {
            const auto time=IsEqualPropertyKey(*key,PKEY_ItemDate) ? item_date(item) : recent_time(item);
            if(!time.dwLowDateTime && !time.dwHighDateTime) return S_OK;
            FILETIME local{}; SYSTEMTIME system{}; DATE date{};
            check_win32(FileTimeToLocalFileTime(&time,&local)); check_win32(FileTimeToSystemTime(&local,&system));
            if(!SystemTimeToVariantTime(&system,&date)) return E_INVALIDARG;
            value->vt=VT_DATE; value->date=date; return S_OK;
        }
        if(IsEqualPropertyKey(*key,PKEY_ItemTypeText))
            return InitVariantFromString(detail(item,1).c_str(),value);
        if(IsEqualPropertyKey(*key,PKEY_ItemPathDisplay))
            return InitVariantFromString(detail(item,2).c_str(),value);
        if(IsEqualPropertyKey(*key,PKEY_Comment))
            return InitVariantFromString(detail(item,4).c_str(),value);
        // Keep binary/property-store data typed. An unavailable property is empty,
        // rather than a serialized value coerced into the item's visible text.
        auto target=navigation_target(item);
        ComPtr<IShellFolder2> parent;
        PCUITEMID_CHILD child=nullptr;
        const auto bound=SHBindToParent(target.get(),IID_PPV_ARGS(&parent),&child);
        if(FAILED(bound)) return S_OK;
        const auto status=parent->GetDetailsEx(child,key,value);
        if(FAILED(status)) { VariantClear(value); VariantInit(value); return S_OK; }
        return S_OK;
    });
}
HRESULT ShellFolder::GetDetailsOf(PCUITEMID_CHILD pidl,UINT column,SHELLDETAILS* result) noexcept
{
    if(!result) return E_POINTER;
    *result={};
    return guarded([&]{
        if(column>7) return E_INVALIDARG;
        result->fmt=LVCFMT_LEFT; result->cxChar=column==2 ? 60 : 28;
        if(pidl) return make_string(detail(read_item_id(pidl),column),&result->str);
        constexpr UINT labels[]{IDS_COLUMN_NAME,IDS_SECTION,IDS_LOCATION,IDS_RECENT_DATE,IDS_RECENT_PINNED,
            IDS_EXTENSION,IDS_FAMILY,IDS_ITEM_DATE};
        return make_string(text(labels[column]),&result->str);
    });
}
HRESULT ShellFolder::MapColumnToSCID(UINT column,SHCOLUMNID* key) noexcept
{
    if(!key) return E_POINTER;
    *key={};
    if(column>7) return E_INVALIDARG;
    const PROPERTYKEY keys[]{PKEY_ItemNameDisplay,PKEY_ItemTypeText,PKEY_ItemPathDisplay,PKEY_DateModified,PKEY_Comment,
        PKEY_FileExtension,PKEY_KindText,PKEY_ItemDate};
    *key=keys[column];
    return S_OK;
}
HRESULT ShellFolder::MessageSFVCB(UINT message,WPARAM first,LPARAM second) noexcept
{
    switch(message)
    {
    case SFVM_DEFVIEWMODE:
        if(!second) return E_POINTER;
        *reinterpret_cast<FOLDERVIEWMODE*>(second)=FVM_ICON; return S_OK;
    case SFVM_WINDOWCREATED: view_window_=reinterpret_cast<HWND>(first); return S_OK;
    case SFVM_GETNOTIFY:
        if(!first || !second) return E_POINTER;
        *reinterpret_cast<PCIDLIST_ABSOLUTE*>(first)=root_.get();
        *reinterpret_cast<LONG*>(second)=SHCNE_ALLEVENTS; return S_OK;
    default: return E_NOTIMPL;
    }
}
HRESULT ShellFolder::CallBack(IShellFolder*,HWND owner,IDataObject*,UINT message,WPARAM first,LPARAM second) noexcept
{
    const bool is_invocation = message == DFM_INVOKECOMMAND || message == DFM_INVOKECOMMANDEX;
    const auto* invoked_command = is_invocation ? background_command(first) : nullptr;

    const auto status = guarded([&] {
        if (message == DFM_MERGECONTEXTMENU)
        {
            auto* menu = reinterpret_cast<QCMINFO*>(second);
            if (!menu) return E_POINTER;
            if (first & CMF_DEFAULTONLY) return S_OK;


            for (const auto& command : BackgroundCommands)
            {
                if (menu->idCmdFirst > menu->idCmdLast) return HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);

                const UINT flags = MF_BYPOSITION | MF_STRING;

                const UINT command_id = menu->idCmdFirst++;
                const auto label = text(command.label);
                check_win32(InsertMenuW(menu->hmenu, menu->indexMenu++, flags, command_id, label.c_str()));
                menu_icons_.attach(menu->hmenu, command_id, command.glyph, owner);
            }
            return S_OK;
        }

        if (is_invocation)
        {
            if (!invoked_command) return S_FALSE;
            return invoke_background_command(owner, root_.get(), invoked_command->action);
        }

        return E_NOTIMPL;
    });

    if (FAILED(status) && invoked_command)
        report_error(owner, status, invoked_command->error_label);
    return status;
}
HRESULT ShellFolder::DragEnter(IDataObject* data,DWORD,POINTL,DWORD* effect) noexcept
{
    if (!effect) return E_POINTER;
    ComPtr<IShellItemArray> items;
    acceptable_drop_ = data && SUCCEEDED(SHCreateShellItemArrayFromDataObject(data, IID_PPV_ARGS(&items)));
    *effect = acceptable_drop_ ? drop_effect(*effect) : DROPEFFECT_NONE;
    return S_OK;
}
HRESULT ShellFolder::DragOver(DWORD,POINTL,DWORD* effect) noexcept
{
    if (!effect) return E_POINTER;
    *effect = acceptable_drop_ ? drop_effect(*effect) : DROPEFFECT_NONE;
    return S_OK;
}

HRESULT ShellFolder::DragLeave() noexcept
{
    acceptable_drop_ = false;
    return S_OK;
}

HRESULT ShellFolder::Drop(IDataObject* data,DWORD,POINTL,DWORD* effect) noexcept
{
    if (!effect) return E_POINTER;

    const DWORD accepted = drop_effect(*effect);
    *effect = DROPEFFECT_NONE;
    acceptable_drop_ = false;

    const auto status = guarded([&] {
        if (!data || !accepted) return E_INVALIDARG;
        ComPtr<IShellItemArray> items;
        check(SHCreateShellItemArrayFromDataObject(data, IID_PPV_ARGS(&items)));

        const auto direct_status = add_dropped_folders_to_this_pc(view_window_, items.Get());
        if (direct_status != S_FALSE)
        {
            check(direct_status);
            *effect = accepted;
            return S_OK;
        }

        DWORD count{};
        check(items->GetCount(&count));
        for (DWORD index = 0; index < count; ++index)
        {
            ComPtr<IShellItem> item;
            check(items->GetItemAt(index, &item));
            add_favorite(item.Get());
        }

        *effect = accepted;
        return S_OK;
    });

    if (FAILED(status)) report_error(view_window_, status, IDS_ERROR_ADD);
    return status;
}
HRESULT ShellFolder::GetColumnPropertyList(REFIID iid,void** result) noexcept
{
    if(!result) return E_POINTER;
    *result=nullptr;
    // The '0' proplist flag makes each column visible by default. An unflagged
    // entry is available to the property system but hidden in DefView's UI.
    return PSGetPropertyDescriptionListFromString(L"prop:0System.ItemNameDisplay;0System.ItemTypeText;0System.ItemPathDisplay;0System.DateModified;0System.Comment;System.FileExtension;System.KindText;System.ItemDate",iid,result);
}
HRESULT ShellFolder::GetGroupByProperty(PROPERTYKEY* key,BOOL* ascending) noexcept
{ if(!key || !ascending) return E_POINTER;
  return guarded([&]{*key=group_key(view_settings().grouping); *ascending=TRUE; return S_OK;}); }
HRESULT ShellFolder::GetViewMode(FOLDERLOGICALVIEWMODE* mode) noexcept
{ if(!mode) return E_POINTER; *mode=FLVM_ICONS; return S_OK; }
HRESULT ShellFolder::GetIconSize(UINT* size) noexcept
{ if(!size) return E_POINTER; *size=96; return S_OK; }
HRESULT ShellFolder::GetFolderFlags(FOLDERFLAGS* mask,FOLDERFLAGS* flags) noexcept
{ if(!mask || !flags) return E_POINTER; *mask=static_cast<FOLDERFLAGS>(FWF_AUTOARRANGE|FWF_NOENUMREFRESH); *flags=FWF_AUTOARRANGE; return S_OK; }
HRESULT ShellFolder::GetSortColumns(SORTCOLUMN* columns,UINT capacity,UINT* count) noexcept
{
    if(!count) return E_POINTER;
    *count=0;
    if(!columns || !capacity) return E_INVALIDARG;
    return guarded([&]{
        const auto settings=view_settings();
        const auto requested = sort_columns(settings);
        columns[0] = requested[0];
        *count = 1;

        if (capacity > 1 && settings.order == ItemOrder::date)
        {
            columns[1] = requested[1];
            *count = 2;
        }
        return S_OK;
    });
}
HRESULT ShellFolder::GetGroupSubsetCount(UINT* count) noexcept
{ if(!count) return E_POINTER; *count=0; return E_NOTIMPL; }
HRESULT create_shell_folder(REFIID iid,void** result) noexcept
{
    if(!result) return E_POINTER;
    *result=nullptr;
    auto* folder=new(std::nothrow) ShellFolder();
    if(!folder) return E_OUTOFMEMORY;
    const auto status=folder->QueryInterface(iid,result); folder->Release(); return status;
}
}
