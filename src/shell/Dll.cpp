// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; COM DLL exports and class factory; 2026-10-11.
 * Human guidance: Umberto Giacobbi; native C++, explicit validation evidence.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#include "Module.hpp"
#include "../ProjectIdentity.hpp"
#include "../core/Section.hpp"

#include <Unknwn.h>
#include <new>

namespace mcos::shell
{
namespace
{
class ClassFactory final : public IClassFactory
{
public:
    ClassFactory() noexcept { ++live_objects; }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void** result) noexcept override
    {
        if (!result) return E_POINTER;
        *result = nullptr;
        if (iid != IID_IUnknown && iid != IID_IClassFactory) return E_NOINTERFACE;
        *result = static_cast<IClassFactory*>(this);
        AddRef();
        return S_OK;
    }

    ULONG STDMETHODCALLTYPE AddRef() noexcept override { return ++references_; }

    ULONG STDMETHODCALLTYPE Release() noexcept override
    {
        const ULONG remaining = --references_;
        if (remaining == 0) delete this;
        return remaining;
    }

    // outer: aggregation is unsupported.
    // iid: requested Shell folder interface.
    // result: receives one owned reference, or null on failure.
    HRESULT STDMETHODCALLTYPE CreateInstance(IUnknown* outer, REFIID iid, void** result) noexcept override
    {
        if (!result) return E_POINTER;
        *result = nullptr;
        if (outer) return CLASS_E_NOAGGREGATION;
        return create_shell_folder(iid, result);
    }

    HRESULT STDMETHODCALLTYPE LockServer(BOOL lock) noexcept override
    {
        if (lock)
        {
            ++server_locks;
            return S_OK;
        }

        long current = server_locks.load();
        while (current > 0)
        {
            if (server_locks.compare_exchange_weak(current, current - 1)) return S_OK;
        }
        return E_UNEXPECTED;
    }

private:
    ~ClassFactory() { --live_objects; }
    std::atomic<ULONG> references_{1};
};
}
}

// clsid: only the launcher root is implemented.
// iid: requested factory interface.
// result: receives one reference; initialized to null for every failure.
extern "C" HRESULT __stdcall DllGetClassObject(REFCLSID clsid, REFIID iid, void** result)
{
    if (!result) return E_POINTER;
    *result = nullptr;
    if (clsid != mcos::identity::launcher_clsid) return CLASS_E_CLASSNOTAVAILABLE;

    auto* factory = new (std::nothrow) mcos::shell::ClassFactory();
    if (!factory) return E_OUTOFMEMORY;
    const HRESULT status = factory->QueryInterface(iid, result);
    factory->Release();
    return status;
}

extern "C" HRESULT __stdcall DllCanUnloadNow()
{
    return mcos::shell::live_objects.load() == 0 && mcos::shell::server_locks.load() == 0
        ? S_OK : S_FALSE;
}
