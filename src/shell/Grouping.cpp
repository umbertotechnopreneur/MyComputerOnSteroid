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
#include <Propkey.h>
#include <Strsafe.h>
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
class Categories final : public ICategoryProvider, public ICategorizer
{
public:
    Categories() noexcept { ++live_objects; }
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
    { return key && IsEqualPropertyKey(*key,PKEY_ItemTypeText) ? S_OK : S_FALSE; }
    HRESULT STDMETHODCALLTYPE GetDefaultCategory(GUID* id, SHCOLUMNID* key) noexcept override
    {
        if (!id || !key) return E_POINTER;
        *id=category_id; *key=PKEY_ItemTypeText; return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetCategoryForSCID(const SHCOLUMNID* key,GUID* id) noexcept override
    {
        if (!id) return E_POINTER;
        *id=GUID_NULL;
        if (CanCategorizeOnSCID(key)!=S_OK) return E_INVALIDARG;
        *id=category_id; return S_OK;
    }
    HRESULT STDMETHODCALLTYPE EnumCategories(IEnumGUID** result) noexcept override
    { if (!result) return E_POINTER; *result=nullptr; return S_FALSE; }
    HRESULT STDMETHODCALLTYPE GetCategoryName(const GUID* id,PWSTR value,UINT size) noexcept override
    {
        if (!id || !value) return E_POINTER;
        if (*id!=category_id) return E_INVALIDARG;
        return GetDescription(value,size);
    }
    HRESULT STDMETHODCALLTYPE CreateCategory(const GUID* id,REFIID iid,void** value) noexcept override
    {
        if (!value) return E_POINTER;
        *value=nullptr;
        if (!id || *id!=category_id) return E_INVALIDARG;
        return QueryInterface(iid,value);
    }
    HRESULT STDMETHODCALLTYPE GetDescription(PWSTR value,UINT size) noexcept override
    { return guarded([&]{ return StringCchCopyW(value,size,text(IDS_SECTION).c_str()); }); }
    HRESULT STDMETHODCALLTYPE GetCategory(UINT count,PCUITEMID_CHILD_ARRAY items,DWORD* ids) noexcept override
    {
        return guarded([&]{
            if (!items || !ids) return E_POINTER;
            for(UINT i=0;i<count;++i) ids[i]=static_cast<DWORD>(read_item_id(items[i]).section);
            return S_OK;
        });
    }
    HRESULT STDMETHODCALLTYPE GetCategoryInfo(DWORD id,CATEGORY_INFO* info) noexcept override
    {
        return guarded([&]{
            if (!info) return E_POINTER;
            *info={};
            if(id<1 || id>5) return E_INVALIDARG;
            info->cif=CATINFO_NORMAL;
            return StringCchCopyW(info->wszName,ARRAYSIZE(info->wszName),text(section_label(static_cast<Section>(id))).c_str());
        });
    }
    HRESULT STDMETHODCALLTYPE CompareCategory(CATSORT_FLAGS,DWORD first,DWORD second) noexcept override
    {
        const short order=first<second ? -1 : first>second ? 1 : 0;
        return MAKE_HRESULT(SEVERITY_SUCCESS,0,static_cast<USHORT>(order));
    }
private:
    ~Categories() { --live_objects; }
    std::atomic<ULONG> references_{1};
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
