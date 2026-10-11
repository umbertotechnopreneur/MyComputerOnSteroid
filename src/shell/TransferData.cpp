// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; native file-transfer adapter; 2026-10-12.
 * Human guidance: Umberto Giacobbi; drag documents to external applications.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#include "ItemId.hpp"
#include "Module.hpp"
#include "../core/ViewSettings.hpp"
#include <algorithm>

namespace mcos::shell
{
namespace
{
bool file_format(CLIPFORMAT format) noexcept
{
    if(format==CF_HDROP) return true;
    wchar_t name[128]{};
    if(!GetClipboardFormatNameW(format,name,ARRAYSIZE(name))) return false;
    return !wcscmp(name,CFSTR_FILENAMEA) || !wcscmp(name,CFSTR_FILENAMEW) ||
        !wcscmp(name,CFSTR_FILEDESCRIPTORA) || !wcscmp(name,CFSTR_FILEDESCRIPTORW) ||
        !wcscmp(name,CFSTR_FILECONTENTS);
}

ComPtr<IDataObject> shell_data(std::vector<PCIDLIST_ABSOLUTE>& ids)
{
    ComPtr<IShellItemArray> selection;
    check(SHCreateShellItemArrayFromIDLists(static_cast<UINT>(ids.size()),ids.data(),&selection));
    ComPtr<IDataObject> data;
    check(selection->BindToHandler(nullptr,BHID_DataObject,IID_PPV_ARGS(&data)));
    return data;
}

class TransferData final : public IDataObject
{
public:
    TransferData(ComPtr<IDataObject> references,ComPtr<IDataObject> files)
        : references_(std::move(references)),files_(std::move(files)),
          preferred_(static_cast<CLIPFORMAT>(RegisterClipboardFormatW(CFSTR_PREFERREDDROPEFFECT)))
    { ++live_objects; }
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid,void** result) noexcept override
    {
        if(!result) return E_POINTER;
        *result=nullptr;
        if(iid!=IID_IUnknown && iid!=IID_IDataObject) return E_NOINTERFACE;
        *result=static_cast<IDataObject*>(this); AddRef(); return S_OK;
    }
    ULONG STDMETHODCALLTYPE AddRef() noexcept override { return ++count_; }
    ULONG STDMETHODCALLTYPE Release() noexcept override
    { const auto left=--count_; if(!left) delete this; return left; }
    HRESULT STDMETHODCALLTYPE GetData(FORMATETC* format,STGMEDIUM* medium) noexcept override
    {
        if(!format || !medium) return E_POINTER;
        *medium={};
        if(format->cfFormat!=preferred_) return source(format)->GetData(format,medium);
        if(!(format->tymed & TYMED_HGLOBAL)) return DV_E_TYMED;
        if(format->dwAspect!=DVASPECT_CONTENT) return DV_E_DVASPECT;
        if(format->lindex!=-1) return DV_E_LINDEX;
        auto memory=GlobalAlloc(GMEM_MOVEABLE,sizeof(DWORD));
        if(!memory) return E_OUTOFMEMORY;
        auto* effect=static_cast<DWORD*>(GlobalLock(memory));
        if(!effect) { GlobalFree(memory); return E_OUTOFMEMORY; }
        *effect=DROPEFFECT_COPY; GlobalUnlock(memory);
        medium->tymed=TYMED_HGLOBAL; medium->hGlobal=memory; return S_OK;
    }
    HRESULT STDMETHODCALLTYPE GetDataHere(FORMATETC* format,STGMEDIUM* medium) noexcept override
    {
        if(!format || !medium) return E_POINTER;
        return format->cfFormat==preferred_ ? DV_E_FORMATETC : source(format)->GetDataHere(format,medium);
    }
    HRESULT STDMETHODCALLTYPE QueryGetData(FORMATETC* format) noexcept override
    {
        if(!format) return E_POINTER;
        if(format->cfFormat!=preferred_) return source(format)->QueryGetData(format);
        if(!(format->tymed & TYMED_HGLOBAL)) return DV_E_TYMED;
        if(format->dwAspect!=DVASPECT_CONTENT) return DV_E_DVASPECT;
        return format->lindex==-1 ? S_OK : DV_E_LINDEX;
    }
    HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc(FORMATETC* input,FORMATETC* output) noexcept override
    {
        if(!input || !output) return E_POINTER;
        output->ptd=nullptr;
        return input->cfFormat==preferred_ ? DATA_S_SAMEFORMATETC : source(input)->GetCanonicalFormatEtc(input,output);
    }
    HRESULT STDMETHODCALLTYPE SetData(FORMATETC* format,STGMEDIUM* medium,BOOL release) noexcept override
    {
        if(!format || !medium) return E_POINTER;
        // Destination feedback belongs to the references, never to document
        // targets. Do not replace the read-only exported identity or copy hint.
        if(file_format(format->cfFormat) || format->cfFormat==preferred_) return E_ACCESSDENIED;
        return references_->SetData(format,medium,release);
    }
    HRESULT STDMETHODCALLTYPE EnumFormatEtc(DWORD direction,IEnumFORMATETC** result) noexcept override
    {
        if(!result) return E_POINTER;
        *result=nullptr;
        if(direction!=DATADIR_GET) return references_->EnumFormatEtc(direction,result);
        return guarded([&]{
            std::vector<FORMATETC> formats;
            for(const bool targets:{false,true})
            {
                ComPtr<IEnumFORMATETC> enumerator;
                check((targets ? files_ : references_)->EnumFormatEtc(DATADIR_GET,&enumerator));
                FORMATETC format{};
                HRESULT status;
                while((status=enumerator->Next(1,&format,nullptr))==S_OK)
                {
                    // The standard filesystem formats used here have no target
                    // device; omit device-specific entries instead of losing it.
                    const bool device=format.ptd!=nullptr;
                    CoTaskMemFree(format.ptd); format.ptd=nullptr;
                    if(device || format.cfFormat==preferred_ || file_format(format.cfFormat)!=targets) continue;
                    formats.push_back(format);
                }
                check(status);
            }
            formats.push_back({preferred_,nullptr,DVASPECT_CONTENT,-1,TYMED_HGLOBAL});
            return SHCreateStdEnumFmtEtc(static_cast<UINT>(formats.size()),formats.data(),result);
        });
    }
    HRESULT STDMETHODCALLTYPE DAdvise(FORMATETC*,DWORD,IAdviseSink*,DWORD* connection) noexcept override
    { if(connection) *connection=0; return OLE_E_ADVISENOTSUPPORTED; }
    HRESULT STDMETHODCALLTYPE DUnadvise(DWORD) noexcept override { return OLE_E_ADVISENOTSUPPORTED; }
    HRESULT STDMETHODCALLTYPE EnumDAdvise(IEnumSTATDATA** result) noexcept override
    { if(result) *result=nullptr; return OLE_E_ADVISENOTSUPPORTED; }
private:
    ~TransferData() { --live_objects; }
    IDataObject* source(const FORMATETC* format) const noexcept
    { return file_format(format->cfFormat) ? files_.Get() : references_.Get(); }
    std::atomic<ULONG> count_{1};
    ComPtr<IDataObject> references_,files_;
    CLIPFORMAT preferred_;
};
}

HRESULT transfer_data_object(UINT count,PCUITEMID_CHILD_ARRAY items,IDataObject** result) noexcept
{
    if(!result) return E_POINTER;
    *result=nullptr;
    return guarded([&]{
        if(!count || !items) return E_INVALIDARG;
        std::vector<Pidl> reference_ids,target_ids;
        std::vector<PCIDLIST_ABSOLUTE> references,targets;
        bool documents=false;
        for(UINT index=0;index<count;++index)
        {
            const auto item=read_item_id(items[index]);
            reference_ids.push_back(parse_pidl(item.path)); references.push_back(reference_ids.back().get());
            const bool document=item.section==Section::files || item.section==Section::recents;
            const auto path=document ? item_target_path(item) : item.path;
            // A missing/virtual document must report failure rather than silently
            // exporting its cache hash as though it were the document filename.
            if(document && (path.empty() || !std::filesystem::path(path).is_absolute())) return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
            target_ids.push_back(parse_pidl(path)); targets.push_back(target_ids.back().get());
            documents|=document;
        }
        auto data=shell_data(references);
        if(!documents) { *result=data.Detach(); return S_OK; }
        auto exported=shell_data(targets);
        *result=new TransferData(std::move(data),std::move(exported));
        return S_OK;
    });
}
}
