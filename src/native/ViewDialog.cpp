// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; native searchable collection options; 2026-10-11.
 * Human guidance: Umberto Giacobbi; editable families and native Shell order.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */
#include "ViewDialog.hpp"
#include "../ProjectIdentity.hpp"
#include "../../resources/resource.h"
#include <array>

namespace mcos
{
namespace
{
struct DialogState
{
    ViewSettings* settings;
    std::array<HICON,3> icons{};
    ~DialogState() { for(auto icon:icons) if(icon) DestroyIcon(icon); }
};
void choices(HWND dialog,UINT control,std::initializer_list<UINT> labels,unsigned selected)
{
    for(const auto label:labels)
        if(SendDlgItemMessageW(dialog,control,CB_ADDSTRING,0,reinterpret_cast<LPARAM>(text(label).c_str()))<0) check(E_FAIL);
    SendDlgItemMessageW(dialog,control,CB_SETCURSEL,selected,0);
}
unsigned selection(HWND dialog,UINT control)
{
    const auto value=SendDlgItemMessageW(dialog,control,CB_GETCURSEL,0,0);
    if(value<0) check(E_INVALIDARG);
    return static_cast<unsigned>(value);
}
std::wstring edit(HWND dialog,UINT control)
{
    const auto length=GetWindowTextLengthW(GetDlgItem(dialog,control));
    std::wstring result(static_cast<size_t>(length)+1,L'\0');
    GetDlgItemTextW(dialog,control,result.data(),length+1); result.resize(length); return result;
}
INT_PTR CALLBACK view_dialog(HWND dialog,UINT message,WPARAM first,LPARAM second) noexcept
{
    if(message==WM_INITDIALOG)
    {
        const auto status=guarded([&]{
            auto* state=reinterpret_cast<DialogState*>(second);
            auto* settings=state->settings;
            SetWindowLongPtrW(dialog,DWLP_USER,second);
            const UINT controls[]{IDC_VIEW_SEARCH_ICON,IDC_VIEW_GROUP_ICON,IDC_VIEW_FAMILY_ICON};
            const SHSTOCKICONID stocks[]{SIID_FIND,SIID_FOLDER,SIID_DOCASSOC};
            for(size_t i=0;i<state->icons.size();++i)
            {
                SHSTOCKICONINFO icon{sizeof(icon)};
                if(SUCCEEDED(SHGetStockIconInfo(stocks[i],SHGSI_ICON|SHGSI_SMALLICON,&icon)))
                {
                    state->icons[i]=icon.hIcon;
                    SendDlgItemMessageW(dialog,controls[i],STM_SETICON,reinterpret_cast<WPARAM>(icon.hIcon),0);
                }
            }
            SendDlgItemMessageW(dialog,IDC_VIEW_QUERY,EM_SETLIMITTEXT,256,0);
            SendDlgItemMessageW(dialog,IDC_VIEW_FAMILIES,EM_SETLIMITTEXT,16384,0);
            SetDlgItemTextW(dialog,IDC_VIEW_QUERY,settings->query.c_str());
            SetDlgItemTextW(dialog,IDC_VIEW_FAMILIES,settings->families.c_str());
            choices(dialog,IDC_VIEW_FIELD,{IDS_SEARCH_ANY,IDS_SEARCH_NAME,IDS_SEARCH_PATH,IDS_SEARCH_EXTENSION},static_cast<unsigned>(settings->field));
            choices(dialog,IDC_VIEW_GROUP,{IDS_GROUP_SECTIONS,IDS_GROUP_EXTENSIONS,IDS_GROUP_FAMILIES,IDS_GROUP_NONE},static_cast<unsigned>(settings->grouping));
            choices(dialog,IDC_VIEW_SORT,{IDS_SORT_NAME,IDS_SORT_DATE},static_cast<unsigned>(settings->order));
            choices(dialog,IDC_VIEW_DIRECTION,{IDS_SORT_ASCENDING,IDS_SORT_DESCENDING},settings->descending ? 1u : 0u);
            return S_OK;
        });
        if(FAILED(status)) { report_error(dialog,status,IDS_ERROR_VIEW); EndDialog(dialog,IDCANCEL); }
        return TRUE;
    }
    if(message==WM_CTLCOLORSTATIC)
    {
        const auto control=GetDlgCtrlID(reinterpret_cast<HWND>(second));
        if(control==IDC_VIEW_SEARCH_HINT || control==IDC_VIEW_DATE_HINT || control==IDC_VIEW_FAMILY_HINT)
        {
            SetTextColor(reinterpret_cast<HDC>(first),GetSysColor(COLOR_GRAYTEXT));
            SetBkColor(reinterpret_cast<HDC>(first),GetSysColor(COLOR_3DFACE));
            return reinterpret_cast<INT_PTR>(GetSysColorBrush(COLOR_3DFACE));
        }
    }
    if(message!=WM_COMMAND) return FALSE;
    if(LOWORD(first)==IDCANCEL) { EndDialog(dialog,IDCANCEL); return TRUE; }
    if(LOWORD(first)==IDC_VIEW_CLEAR) { SetDlgItemTextW(dialog,IDC_VIEW_QUERY,L""); SetFocus(GetDlgItem(dialog,IDC_VIEW_QUERY)); return TRUE; }
    if(LOWORD(first)!=IDOK) return FALSE;
    const auto status=guarded([&]{
        auto* state=reinterpret_cast<DialogState*>(GetWindowLongPtrW(dialog,DWLP_USER));
        if(!state || !state->settings) return E_UNEXPECTED;
        auto* settings=state->settings;
        auto pending=*settings;
        pending.query=edit(dialog,IDC_VIEW_QUERY); pending.families=edit(dialog,IDC_VIEW_FAMILIES);
        if(FAILED(guarded([&]{parse_families(pending.families); return S_OK;})))
        {
            MessageBoxW(dialog,text(IDS_INVALID_FAMILIES).c_str(),identity::launcher_name,MB_OK|MB_ICONWARNING);
            SetFocus(GetDlgItem(dialog,IDC_VIEW_FAMILIES)); return S_FALSE;
        }
        pending.field=static_cast<SearchField>(selection(dialog,IDC_VIEW_FIELD));
        pending.grouping=static_cast<GroupMode>(selection(dialog,IDC_VIEW_GROUP));
        pending.order=static_cast<ItemOrder>(selection(dialog,IDC_VIEW_SORT));
        pending.descending=selection(dialog,IDC_VIEW_DIRECTION)!=0;
        *settings=std::move(pending); return S_OK;
    });
    if(FAILED(status)) report_error(dialog,status,IDS_ERROR_VIEW);
    else if(status==S_OK) EndDialog(dialog,IDOK);
    return TRUE;
}
}
bool choose_view_settings(HWND owner,ViewSettings& settings)
{
    HMODULE module=nullptr;
    check_win32(GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
        reinterpret_cast<PCWSTR>(&identity::launcher_clsid),&module));
    DialogState state{&settings};
    const auto result=DialogBoxParamW(module,MAKEINTRESOURCEW(IDD_VIEW_OPTIONS),owner,view_dialog,reinterpret_cast<LPARAM>(&state));
    if(result==-1) check(HRESULT_FROM_WIN32(GetLastError()));
    return result==IDOK;
}
}
