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

#include "Grouping.hpp"
#include "ItemId.hpp"
#include "Module.hpp"
#include "../core/ViewSettings.hpp"
#include <Propkey.h>
#include <Strsafe.h>
#include <algorithm>
#include <mutex>
#include "../../resources/resource.h"

namespace mcos::shell
{
UINT section_label(Section section) noexcept
{
    switch (section)
    {
    case Section::folders: return IDS_FOLDERS;
    case Section::drives: return IDS_DRIVES;
    case Section::apps: return IDS_APPS;
    case Section::files: return IDS_FILES;
    case Section::recents: return IDS_RECENTS;
    }
    return IDS_SECTION;
}
namespace
{
constexpr GUID category_id{0xb36b9212,0x2d35,0x4e65,{0x8a,0x97,0x0a,0xa5,0xd2,0x80,0xe1,0x58}};
constexpr GUID extension_id{0xb36b9212,0x2d35,0x4e65,{0x8a,0x97,0x0a,0xa5,0xd2,0x80,0xe1,0x59}};
constexpr GUID family_id{0xb36b9212,0x2d35,0x4e65,{0x8a,0x97,0x0a,0xa5,0xd2,0x80,0xe1,0x5a}};
GUID category_guid(GroupMode mode) noexcept
{ return mode==GroupMode::extension ? extension_id : mode==GroupMode::families ? family_id : category_id; }
PROPERTYKEY category_key(GroupMode mode) noexcept
{ return mode==GroupMode::extension ? PKEY_FileExtension : mode==GroupMode::families ? PKEY_KindText : PKEY_ItemTypeText; }
class Categories final : public ICategoryProvider, public ICategorizer
{
public:
    explicit Categories(GroupMode mode=GroupMode::sections) noexcept : mode_(mode) { ++live_objects; }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** value) noexcept override
    {
        if (!value) return E_POINTER;
        *value = nullptr;
        if (iid == IID_IUnknown || iid == IID_ICategoryProvider) *value = static_cast<ICategoryProvider*>(this);
        else if (iid == IID_ICategorizer) *value = static_cast<ICategorizer*>(this);
        else return E_NOINTERFACE;
        AddRef(); return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() noexcept override { return ++references_; }
    ULONG STDMETHODCALLTYPE Release() noexcept override
    { auto left=--references_; if (!left) delete this; return left; }
    HRESULT STDMETHODCALLTYPE CanCategorizeOnSCID(const SHCOLUMNID* key) noexcept override
    { return key && (IsEqualPropertyKey(*key,PKEY_ItemTypeText) || IsEqualPropertyKey(*key,PKEY_FileExtension) ||
        IsEqualPropertyKey(*key,PKEY_KindText)) ? S_OK : S_FALSE; }
    HRESULT STDMETHODCALLTYPE GetDefaultCategory(GUID* id, SHCOLUMNID* key) noexcept override
    {
        if (!id || !key) return E_POINTER;
        return guarded([&]{ const auto mode=view_settings().grouping;
            *id=mode==GroupMode::none ? GUID_NULL : category_guid(mode);
            *key=mode==GroupMode::none ? PKEY_Null : category_key(mode); return S_OK; });
    }
    HRESULT STDMETHODCALLTYPE GetCategoryForSCID(const SHCOLUMNID* key,GUID* id) noexcept override
    {
        if (!id) return E_POINTER;
        *id=GUID_NULL;
        if (CanCategorizeOnSCID(key)!=S_OK) return E_INVALIDARG;
        *id=IsEqualPropertyKey(*key,PKEY_FileExtension) ? extension_id :
            IsEqualPropertyKey(*key,PKEY_KindText) ? family_id : category_id; return S_OK;
    }
    HRESULT STDMETHODCALLTYPE EnumCategories(IEnumGUID** result) noexcept override
    { if (!result) return E_POINTER; *result=nullptr; return S_FALSE; }
    HRESULT STDMETHODCALLTYPE GetCategoryName(const GUID* id,PWSTR value,UINT size) noexcept override
    {
        if (!id || !value) return E_POINTER;
        if (*id!=category_id && *id!=extension_id && *id!=family_id) return E_INVALIDARG;
        return guarded([&]{return StringCchCopyW(value,size,text(*id==extension_id ? IDS_EXTENSION :
            *id==family_id ? IDS_FAMILY : IDS_SECTION).c_str());});
    }
    HRESULT STDMETHODCALLTYPE CreateCategory(const GUID* id,REFIID iid,void** value) noexcept override
    {
        if (!value) return E_POINTER;
        *value=nullptr;
        if (!id || (*id!=category_id && *id!=extension_id && *id!=family_id)) return E_INVALIDARG;
        return guarded([&]{
            auto* object=new Categories(*id==extension_id ? GroupMode::extension :
                *id==family_id ? GroupMode::families : GroupMode::sections);
            const auto status=object->QueryInterface(iid,value); object->Release(); return status;
        });
    }
    HRESULT STDMETHODCALLTYPE GetDescription(PWSTR value,UINT size) noexcept override
    { return guarded([&]{ return StringCchCopyW(value,size,text(mode_==GroupMode::extension ? IDS_EXTENSION :
        mode_==GroupMode::families ? IDS_FAMILY : IDS_SECTION).c_str()); }); }
    HRESULT STDMETHODCALLTYPE GetCategory(UINT count,PCUITEMID_CHILD_ARRAY items,DWORD* ids) noexcept override
    {
        return guarded([&]{
            if (!items || !ids) return E_POINTER;
            const auto families=mode_==GroupMode::families ? parse_families(view_settings().families) : std::vector<ExtensionFamily>{};
            std::lock_guard lock(mutex_);
            for(UINT i=0;i<count;++i)
            {
                const auto item=read_item_id(items[i]);
                if(mode_==GroupMode::sections || item.section==Section::folders || item.section==Section::drives || item.section==Section::apps)
                { ids[i]=static_cast<DWORD>(item.section); continue; }
                std::wstring name;
                const auto status=guarded([&]{
                    name=mode_==GroupMode::families ? item_family(item,families) : item_extension(item);
                    return S_OK;
                });
                if(FAILED(status) || name.empty()) name=text(IDS_NO_EXTENSION);
                const auto found=std::find(names_.begin(),names_.end(),name);
                if(found==names_.end()) { names_.push_back(std::move(name)); ids[i]=100+static_cast<DWORD>(names_.size()-1); }
                else ids[i]=100+static_cast<DWORD>(found-names_.begin());
            }
            return S_OK;
        });
    }
    HRESULT STDMETHODCALLTYPE GetCategoryInfo(DWORD id,CATEGORY_INFO* info) noexcept override
    {
        return guarded([&]{
            if (!info) return E_POINTER;
            *info={};
            std::lock_guard lock(mutex_);
            info->cif=CATINFO_NORMAL;
            const auto name=category_name(id);
            return StringCchCopyW(info->wszName,ARRAYSIZE(info->wszName),name.c_str());
        });
    }
    HRESULT STDMETHODCALLTYPE CompareCategory(CATSORT_FLAGS,DWORD first,DWORD second) noexcept override
    {
        return guarded([&]{
            std::lock_guard lock(mutex_);
            int comparison{};
            const auto first_rank=category_rank(first),second_rank=category_rank(second);
            if(first_rank!=second_rank) comparison=first_rank<second_rank ? -1 : 1;
            else comparison=StrCmpLogicalW(category_name(first).c_str(),category_name(second).c_str());
            const short order=comparison<0 ? -1 : comparison>0 ? 1 : 0;
            return MAKE_HRESULT(SEVERITY_SUCCESS,0,static_cast<USHORT>(order));
        });
    }
private:
    ~Categories() { --live_objects; }
    std::atomic<ULONG> references_{1};
    GroupMode mode_;
    std::mutex mutex_;
    std::vector<std::wstring> names_;
    // Category identity remains stable; presentation order is independent of
    // the Section enum and of whichever file happens to enumerate first.
    unsigned category_rank(DWORD id)
    {
        if(id<100)
        {
            category_name(id); // Validate unknown section IDs before comparing.
            switch(static_cast<Section>(id))
            {
            case Section::drives: return 0;
            case Section::folders: return 1;
            case Section::apps: return 2;
            case Section::files: return 3;
            case Section::recents: return 4;
            }
        }
        const auto name=category_name(id);
        if(name==text(IDS_OTHER_EXTENSIONS)) return 100;
        if(name==text(IDS_NO_EXTENSION)) return 101;
        if(mode_==GroupMode::families)
        {
            if(!_wcsicmp(name.c_str(),L"Office")) return 10;
            if(!_wcsicmp(name.c_str(),L"PDF")) return 11;
            if(!_wcsicmp(name.c_str(),L"Sviluppo") || !_wcsicmp(name.c_str(),L"Development")) return 12;
        }
        return 20; // Remaining families/extensions use natural alphabetical order.
    }
    std::wstring category_name(DWORD id)
    {
        if(id>=1 && id<=5) return text(section_label(static_cast<Section>(id)));
        if(id>=100 && id-100<names_.size()) return names_[id-100];
        check(E_INVALIDARG); return {};
    }
};
}
HRESULT create_categories(REFIID iid,void** result) noexcept
{
    if (!result) return E_POINTER;
    *result=nullptr;
    auto* object=new(std::nothrow) Categories();
    if(!object) return E_OUTOFMEMORY;
    auto status=object->QueryInterface(iid,result);
    object->Release(); return status;
}
}
