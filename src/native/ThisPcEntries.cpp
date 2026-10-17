// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; native This PC entry manager; 2026-10-16.
 * Modified with AI: OpenAI Codex; current-format registry management and shared helpers; 2026-10-17.
 * Human guidance: Umberto Giacobbi; native management of folders shown directly in This PC.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#include "ThisPcEntries.hpp"
#include "Support.hpp"
#include "Dialog.hpp"
#include "../../resources/resource.h"
#include <CommCtrl.h>
#include <algorithm>
#include <array>
#include <map>
#include <optional>
#include <utility>
#include <vector>
#include <sddl.h>

namespace mcos
{
namespace
{
constexpr wchar_t CurrentMarker[] = L"MCOS.ThisPcEntry.Managed";
constexpr wchar_t CurrentTarget[] = L"MCOS.ThisPcEntry.TargetPath";
constexpr wchar_t CurrentIcon[] = L"MCOS.ThisPcEntry.Icon";
constexpr wchar_t CurrentOwner[] = L"MyComputerOnSteroid.ThisPcEntry";
constexpr wchar_t ClassesRoot[] = L"Software\\Classes\\CLSID";
constexpr wchar_t NamespaceRoot[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\MyComputer\\NameSpace";
constexpr wchar_t FolderShortcutClass[] = L"{0E5AAE11-A475-4C5B-AB00-C66DE400274E}";

// One owned namespace registration. The target is a reference to a real folder;
// deleting this record must never delete that folder or its contents.
struct ThisPcEntry
{
    std::wstring id;
    std::wstring name;
    std::wstring target;
    std::wstring icon;
    DWORD sort_order{80};
};

// Owns a single HKEY; subkeys returned by Win32 are closed on every exit path.
class RegistryKey final
{
public:
    RegistryKey() = default;
    explicit RegistryKey(HKEY key) noexcept : key_(key) {}
    RegistryKey(const RegistryKey&) = delete;
    RegistryKey& operator=(const RegistryKey&) = delete;
    RegistryKey(RegistryKey&& other) noexcept : key_(std::exchange(other.key_, nullptr)) {}
    RegistryKey& operator=(RegistryKey&& other) noexcept
    {
        if (this != &other)
        {
            reset();
            key_ = std::exchange(other.key_, nullptr);
        }
        return *this;
    }
    ~RegistryKey() { reset(); }

    HKEY get() const noexcept { return key_; }

private:
    void reset() noexcept
    {
        if (key_) RegCloseKey(key_);
        key_ = nullptr;
    }

    HKEY key_{};
};

struct DialogState
{
    std::vector<ThisPcEntry> entries;
    std::wstring selected_id;
    HIMAGELIST images{};
    HICON preview{};
    bool loading{};

    ~DialogState()
    {
        if (images) ImageList_Destroy(images);
        if (preview) DestroyIcon(preview);
    }
};

std::wstring join_registry_path(const wchar_t* root, const std::wstring& child)
{
    return std::wstring(root) + L"\\" + child;
}

RegistryKey create_registry_key(HKEY root, const std::wstring& path)
{
    HKEY key = nullptr;
    DWORD disposition{};
    check(HRESULT_FROM_WIN32(RegCreateKeyExW(
        root,
        path.c_str(),
        0,
        nullptr,
        REG_OPTION_NON_VOLATILE,
        KEY_READ | KEY_WRITE | KEY_WOW64_64KEY,
        nullptr,
        &key,
        &disposition)));
    return RegistryKey(key);
}

void remove_registry_tree(HKEY root, const std::wstring& path)
{
    const LSTATUS status = RegDeleteTreeW(root, path.c_str());
    if (status != ERROR_FILE_NOT_FOUND && status != ERROR_PATH_NOT_FOUND)
        check(HRESULT_FROM_WIN32(status));
}

void set_registry_string(HKEY key, const wchar_t* name, const std::wstring& value, DWORD type = REG_SZ)
{
    const auto bytes = static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t));
    check(HRESULT_FROM_WIN32(RegSetValueExW(
        key,
        name,
        0,
        type,
        reinterpret_cast<const BYTE*>(value.c_str()),
        bytes)));
}

void set_registry_dword(HKEY key, const wchar_t* name, DWORD value)
{
    check(HRESULT_FROM_WIN32(RegSetValueExW(
        key,
        name,
        0,
        REG_DWORD,
        reinterpret_cast<const BYTE*>(&value),
        sizeof(value))));
}

std::optional<std::wstring> read_registry_string(HKEY root, const std::wstring& path, const wchar_t* name)
{
    DWORD bytes{};
    const DWORD flags = RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ;
    LSTATUS status = RegGetValueW(root, path.c_str(), name, flags, nullptr, nullptr, &bytes);
    if (status == ERROR_FILE_NOT_FOUND || status == ERROR_PATH_NOT_FOUND) return std::nullopt;
    check(HRESULT_FROM_WIN32(status));
    if (!bytes || bytes % sizeof(wchar_t) != 0) check(E_FAIL);

    std::wstring value(bytes / sizeof(wchar_t), L'\0');
    status = RegGetValueW(root, path.c_str(), name, flags, nullptr, value.data(), &bytes);
    check(HRESULT_FROM_WIN32(status));
    while (!value.empty() && value.back() == L'\0') value.pop_back();
    return value;
}

std::optional<DWORD> read_registry_dword(HKEY root, const std::wstring& path, const wchar_t* name)
{
    DWORD value{};
    DWORD bytes = sizeof(value);
    const LSTATUS status = RegGetValueW(root, path.c_str(), name, RRF_RT_REG_DWORD, nullptr, &value, &bytes);
    if (status == ERROR_FILE_NOT_FOUND || status == ERROR_PATH_NOT_FOUND) return std::nullopt;
    check(HRESULT_FROM_WIN32(status));
    return value;
}

std::wstring entry_class_path(const std::wstring& id)
{
    return join_registry_path(ClassesRoot, id);
}

std::wstring entry_namespace_path(const std::wstring& id)
{
    return join_registry_path(NamespaceRoot, id);
}

std::wstring new_entry_id()
{
    GUID id{};
    check(CoCreateGuid(&id));
    std::array<wchar_t, 40> text{};
    if (!StringFromGUID2(id, text.data(), static_cast<int>(text.size()))) check(E_FAIL);
    return text.data();
}

std::wstring trim(std::wstring value)
{
    const auto first = value.find_first_not_of(L" \t\r\n");
    if (first == std::wstring::npos) return {};
    const auto last = value.find_last_not_of(L" \t\r\n");
    return value.substr(first, last - first + 1);
}

void set_control_text(HWND dialog, UINT control, const std::wstring& value)
{
    SetDlgItemTextW(dialog, control, value.c_str());
}

bool is_elevated() noexcept
{
    HANDLE raw_token = nullptr;
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &raw_token)) return false;
    TOKEN_ELEVATION elevation{};
    DWORD bytes{};
    const bool elevated = GetTokenInformation(
        raw_token,
        TokenElevation,
        &elevation,
        sizeof(elevation),
        &bytes) && elevation.TokenIsElevated != 0;
    CloseHandle(raw_token);
    return elevated;
}

HRESULT launch_elevated(const std::wstring& parameters)
{
    return launch_process(nullptr, module_path(nullptr), parameters, LaunchMode::elevated);
}

std::wstring current_user_sid()
{
    HANDLE token{};
    check_win32(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token));
    const std::unique_ptr<void, decltype(&CloseHandle)> owned_token(token, CloseHandle);
    DWORD bytes{};
    GetTokenInformation(token, TokenUser, nullptr, 0, &bytes);
    std::vector<BYTE> storage(bytes);
    const BOOL read = GetTokenInformation(token, TokenUser, storage.data(), bytes, &bytes);
    const DWORD error = GetLastError();
    if (!read) check(HRESULT_FROM_WIN32(error));
    wchar_t* raw{};
    check_win32(ConvertSidToStringSidW(reinterpret_cast<TOKEN_USER*>(storage.data())->User.Sid, &raw));
    const std::unique_ptr<wchar_t, decltype(&LocalFree)> owned(raw, LocalFree);
    return raw;
}

// Keep per-user preferences in the normal process. Only a This PC mutation
// requests UAC, and the worker must retain the same Windows user identity.
bool run_elevated_worker(HWND dialog, const std::vector<std::wstring>& arguments)
{
    std::wstring parameters = L"--this-pc-worker " + quote_command_line_argument(current_user_sid());
    for (const auto& argument : arguments)
        parameters += L" " + quote_command_line_argument(argument);
    const auto executable = module_path(nullptr).wstring();
    const HWND owner = GetAncestor(dialog, GA_ROOT);
    SHELLEXECUTEINFOW launch{sizeof(launch)};
    launch.fMask = SEE_MASK_NOCLOSEPROCESS | SEE_MASK_NOASYNC | SEE_MASK_FLAG_NO_UI;
    launch.hwnd = owner;
    launch.lpVerb = L"runas";
    launch.lpFile = executable.c_str();
    launch.lpParameters = parameters.c_str();
    launch.nShow = SW_HIDE;
    if (!ShellExecuteExW(&launch))
    {
        const DWORD error = GetLastError();
        if (error == ERROR_CANCELLED) return false;
        check(HRESULT_FROM_WIN32(error));
    }
    if (!launch.hProcess) check(E_UNEXPECTED);
    const std::unique_ptr<void, decltype(&CloseHandle)> process(launch.hProcess, CloseHandle);
    struct DisabledOwner
    {
        HWND window;
        explicit DisabledOwner(HWND handle) : window(handle) { EnableWindow(window, FALSE); }
        ~DisabledOwner() { EnableWindow(window, TRUE); SetActiveWindow(window); }
    } disabled(owner);
    bool quit{};
    int quit_code{};
    for (;;)
    {
        const DWORD wait = MsgWaitForMultipleObjects(1, &launch.hProcess, FALSE, INFINITE, QS_ALLINPUT);
        if (wait == WAIT_OBJECT_0) break;
        if (wait != WAIT_OBJECT_0 + 1) check(HRESULT_FROM_WIN32(GetLastError()));
        MSG message{};
        while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE))
        {
            if (message.message == WM_QUIT)
            {
                quit = true;
                quit_code = static_cast<int>(message.wParam);
                continue;
            }
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }
    if (quit) PostQuitMessage(quit_code);
    DWORD result{};
    check_win32(GetExitCodeProcess(launch.hProcess, &result));
    check(static_cast<HRESULT>(result));
    return true;
}

std::vector<ThisPcEntry> read_entries_from_hive(HKEY hive)
{
    HKEY raw_namespace = nullptr;
    const LSTATUS opened = RegOpenKeyExW(hive, NamespaceRoot, 0, KEY_READ | KEY_WOW64_64KEY, &raw_namespace);
    if (opened == ERROR_FILE_NOT_FOUND || opened == ERROR_PATH_NOT_FOUND) return {};
    check(HRESULT_FROM_WIN32(opened));
    const RegistryKey namespaces(raw_namespace);

    std::vector<ThisPcEntry> entries;
    for (DWORD index = 0;; ++index)
    {
        std::array<wchar_t, 64> id{};
        DWORD characters = static_cast<DWORD>(id.size());
        const LSTATUS status = RegEnumKeyExW(namespaces.get(), index, id.data(), &characters,
            nullptr, nullptr, nullptr, nullptr);
        if (status == ERROR_NO_MORE_ITEMS) break;
        if (status == ERROR_MORE_DATA) continue;
        check(HRESULT_FROM_WIN32(status));
        if (characters != 38 || id.front() != L'{' || id[characters - 1] != L'}') continue;

        const std::wstring entry_id(id.data(), characters);
        const auto class_path = entry_class_path(entry_id);
        const auto managed = read_registry_string(hive, class_path, CurrentMarker);
        if (!managed || *managed != L"1") continue;

        const auto name = read_registry_string(hive, class_path, nullptr);
        const auto target = read_registry_string(hive, class_path, CurrentTarget);
        const auto icon = read_registry_string(hive, class_path, CurrentIcon);
        if (!name || !target || name->empty() || target->empty()) continue;

        entries.push_back({
            entry_id,
            *name,
            *target,
            icon.value_or(L"%SystemRoot%\\System32\\imageres.dll,-3"),
            read_registry_dword(hive, class_path, L"SortOrderIndex").value_or(80)});
    }
    return entries;
}

std::vector<ThisPcEntry> read_entries()
{
    std::map<std::wstring, ThisPcEntry, std::less<>> unique;
    for (const auto hive : {HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER})
        for (auto entry : read_entries_from_hive(hive))
            unique.try_emplace(entry.id, std::move(entry));

    std::vector<ThisPcEntry> entries;
    entries.reserve(unique.size());
    for (auto& [id, entry] : unique) entries.push_back(std::move(entry));
    std::sort(entries.begin(), entries.end(), [](const ThisPcEntry& left, const ThisPcEntry& right) {
        if (left.sort_order != right.sort_order) return left.sort_order < right.sort_order;
        return _wcsicmp(left.name.c_str(), right.name.c_str()) < 0;
    });
    return entries;
}

bool registry_key_exists(HKEY hive, const std::wstring& path)
{
    HKEY raw = nullptr;
    const LSTATUS status = RegOpenKeyExW(hive, path.c_str(), 0, KEY_READ | KEY_WOW64_64KEY, &raw);
    if (status == ERROR_FILE_NOT_FOUND || status == ERROR_PATH_NOT_FOUND) return false;
    check(HRESULT_FROM_WIN32(status));
    const RegistryKey key(raw);
    return true;
}

// Validate every existing key in both hives before changing either copy. An
// entry owned in HKCU must not grant permission to overwrite an unowned HKLM key.
void validate_entry_ownership(HKEY hive, const ThisPcEntry& entry)
{
    const auto class_path = entry_class_path(entry.id);
    if (registry_key_exists(hive, class_path))
    {
        const auto marker = read_registry_string(hive, class_path, CurrentMarker);
        if (!marker || *marker != L"1") check(E_ACCESSDENIED);
    }

    const auto namespace_path = entry_namespace_path(entry.id);
    if (registry_key_exists(hive, namespace_path))
    {
        const auto owner = read_registry_string(hive, namespace_path, L"MCOSOwner");
        if (!owner || *owner != CurrentOwner) check(E_ACCESSDENIED);
    }
}

void write_entry_to_hive(HKEY hive, const ThisPcEntry& entry)
{
    const auto class_path = entry_class_path(entry.id);
    // Update the current schema in place. Clearing the entire class first was
    // unnecessary and could leave a working entry empty after a failed save.
    const auto class_key = create_registry_key(hive, class_path);
    set_registry_string(class_key.get(), nullptr, entry.name);
    set_registry_string(class_key.get(), L"LocalizedString", entry.name);
    set_registry_dword(class_key.get(), L"DescriptionID", 3);
    set_registry_dword(class_key.get(), L"System.IsPinnedToNameSpaceTree", 1);
    set_registry_dword(class_key.get(), L"SortOrderIndex", entry.sort_order);
    set_registry_string(class_key.get(), CurrentMarker, L"1");
    set_registry_string(class_key.get(), CurrentTarget, entry.target);
    set_registry_string(class_key.get(), CurrentIcon, entry.icon);

    const auto icon_key = create_registry_key(class_key.get(), L"DefaultIcon");
    set_registry_string(icon_key.get(), nullptr, entry.icon);

    const auto server_key = create_registry_key(class_key.get(), L"InprocServer32");
    set_registry_string(server_key.get(), nullptr, L"%SystemRoot%\\System32\\shell32.dll", REG_EXPAND_SZ);
    set_registry_string(server_key.get(), L"ThreadingModel", L"Apartment");

    const auto instance_key = create_registry_key(class_key.get(), L"Instance");
    set_registry_string(instance_key.get(), L"CLSID", FolderShortcutClass);
    const auto property_bag = create_registry_key(instance_key.get(), L"InitPropertyBag");
    set_registry_dword(property_bag.get(), L"Attributes", 0x11);
    set_registry_string(property_bag.get(), L"TargetFolderPath", entry.target, REG_EXPAND_SZ);

    const auto folder_key = create_registry_key(class_key.get(), L"ShellFolder");
    set_registry_dword(folder_key.get(), L"Attributes", 0xF080004D);
    set_registry_dword(folder_key.get(), L"FolderValueFlags", 0x29);
    set_registry_dword(folder_key.get(), L"SortOrderIndex", 0x48);

    const auto namespace_key = create_registry_key(hive, entry_namespace_path(entry.id));
    set_registry_string(namespace_key.get(), nullptr, entry.name);
    set_registry_string(namespace_key.get(), L"MCOSOwner", CurrentOwner);
}

void write_entry(const ThisPcEntry& entry)
{
    for (const auto hive : {HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE}) validate_entry_ownership(hive, entry);
    for (const auto hive : {HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE}) write_entry_to_hive(hive, entry);
}

void remove_entry(const ThisPcEntry& entry)
{
    for (const auto hive : {HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE}) validate_entry_ownership(hive, entry);
    for (const auto hive : {HKEY_CURRENT_USER, HKEY_LOCAL_MACHINE})
    {
        remove_registry_tree(hive, entry_namespace_path(entry.id));
        remove_registry_tree(hive, entry_class_path(entry.id));
    }
}

void notify_this_pc() noexcept
{
    guarded([] {
        const auto this_pc = parse_pidl(L"::{20D04FE0-3AEA-1069-A2D8-08002B30309D}");
        SHChangeNotify(SHCNE_UPDATEDIR, SHCNF_IDLIST | SHCNF_FLUSHNOWAIT, this_pc.get(), nullptr);
        return S_OK;
    });
}

std::optional<std::pair<std::wstring, std::wstring>> choose_folder(HWND owner)
{
    ComPtr<IFileOpenDialog> dialog;
    check(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)));
    FILEOPENDIALOGOPTIONS options{};
    check(dialog->GetOptions(&options));
    check(dialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST | FOS_NOCHANGEDIR));
    const HRESULT status = dialog->Show(owner);
    if (status == HRESULT_FROM_WIN32(ERROR_CANCELLED)) return std::nullopt;
    check(status);

    ComPtr<IShellItem> selected;
    check(dialog->GetResult(&selected));
    return std::pair{display_name(selected.Get(), SIGDN_FILESYSPATH), display_name(selected.Get())};
}

std::optional<std::wstring> choose_icon(HWND owner)
{
    ComPtr<IFileOpenDialog> dialog;
    check(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)));
    FILEOPENDIALOGOPTIONS options{};
    check(dialog->GetOptions(&options));
    check(dialog->SetOptions(options | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_NOCHANGEDIR));
    const auto label = text(IDS_ICON_FILES);
    COMDLG_FILTERSPEC filter[]{ {label.c_str(), L"*.ico"} };
    check(dialog->SetFileTypes(ARRAYSIZE(filter), filter));
    const HRESULT status = dialog->Show(owner);
    if (status == HRESULT_FROM_WIN32(ERROR_CANCELLED)) return std::nullopt;
    check(status);

    ComPtr<IShellItem> selected;
    check(dialog->GetResult(&selected));
    return display_name(selected.Get(), SIGDN_FILESYSPATH) + L",0";
}

// Extract at the monitor's actual size. Handles are owned by the caller; image
// lists copy their pixels. Unavailable decoration never blocks entry management.
HICON entry_icon(const std::wstring& location, int pixels)
{
    std::array<wchar_t, 32768> path{};
    const DWORD length = ExpandEnvironmentStringsW(location.c_str(), path.data(), static_cast<DWORD>(path.size()));
    HICON icon{};
    if (length && length <= path.size())
    {
        const int index = PathParseIconLocationW(path.data());
        if (!PathIsNetworkPathW(path.data()))
            SHDefExtractIconW(path.data(), index, 0, &icon, nullptr, MAKELONG(pixels, 0));
    }
    if (icon) return icon;
    SHSTOCKICONINFO stock{sizeof(stock)};
    if (SUCCEEDED(SHGetStockIconInfo(SIID_FOLDER, SHGSI_ICON | SHGSI_LARGEICON, &stock)))
    {
        icon = reinterpret_cast<HICON>(CopyImage(stock.hIcon, IMAGE_ICON, pixels, pixels, 0));
        DestroyIcon(stock.hIcon);
    }
    return icon;
}

void update_editor(HWND dialog, DialogState& state)
{
    if (state.loading) return;
    const bool editing = !state.selected_id.empty();
    const bool has_target = !dialog_text(dialog, IDC_THIS_PC_TARGET).empty();
    const bool has_name = !trim(dialog_text(dialog, IDC_THIS_PC_NAME)).empty();
    SetDlgItemTextW(dialog, IDC_THIS_PC_MODE, text(editing ? IDS_EDIT_ENTRY : IDS_NEW_ENTRY).c_str());
    SetDlgItemTextW(dialog, IDC_THIS_PC_SAVE, text(editing ? IDS_SAVE_ENTRY : IDS_ADD_ENTRY).c_str());
    EnableWindow(GetDlgItem(dialog, IDC_THIS_PC_REMOVE), editing);
    EnableWindow(GetDlgItem(dialog, IDC_THIS_PC_SAVE), has_target && has_name);
    EnableWindow(GetDlgItem(dialog, IDC_THIS_PC_CHOOSE_ICON), has_target);
}

void update_preview(HWND dialog, DialogState& state)
{
    const int pixels = MulDiv(32, static_cast<int>(GetDpiForWindow(dialog)), 96);
    const HICON icon = entry_icon(dialog_text(dialog, IDC_THIS_PC_ICON), pixels);
    SendDlgItemMessageW(dialog, IDC_THIS_PC_PREVIEW, STM_SETICON, reinterpret_cast<WPARAM>(icon), 0);
    if (state.preview) DestroyIcon(state.preview);
    state.preview = icon;
}

void clear_editor(HWND dialog, DialogState& state)
{
    state.loading = true;
    state.selected_id.clear();
    set_control_text(dialog, IDC_THIS_PC_NAME, L"");
    set_control_text(dialog, IDC_THIS_PC_TARGET, L"");
    set_control_text(dialog, IDC_THIS_PC_ICON, L"%SystemRoot%\\System32\\imageres.dll,-3");
    ListView_SetItemState(GetDlgItem(dialog, IDC_THIS_PC_LIST), -1, 0, LVIS_SELECTED | LVIS_FOCUSED);
    state.loading = false;
    update_editor(dialog, state);
    update_preview(dialog, state);
}

void reload_entries(HWND dialog, DialogState& state, const std::wstring& selected_id = {})
{
    state.entries = read_entries();
    state.loading = true;
    const HWND list = GetDlgItem(dialog, IDC_THIS_PC_LIST);
    ListView_DeleteAllItems(list);
    const int dpi = static_cast<int>(GetDpiForWindow(dialog));
    const int pixels = MulDiv(96, dpi, 96);
    const HIMAGELIST images = ImageList_Create(pixels, pixels, ILC_COLOR32 | ILC_MASK, 1, 8);
    if (!images) check(E_OUTOFMEMORY);
    ListView_SetImageList(list, images, LVSIL_NORMAL);
    if (state.images) ImageList_Destroy(state.images);
    state.images = images;
    ListView_SetExtendedListViewStyle(list, LVS_EX_DOUBLEBUFFER | LVS_EX_BORDERSELECT);
    ListView_SetIconSpacing(list, MulDiv(144, dpi, 96), MulDiv(136, dpi, 96));
    for (size_t index = 0; index < state.entries.size(); ++index)
    {
        const auto& entry = state.entries[index];
        const HICON icon = entry_icon(entry.icon, pixels);
        const int image = icon ? ImageList_AddIcon(images, icon) : -1;
        if (icon) DestroyIcon(icon);
        LVITEMW item{};
        item.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_PARAM;
        item.iItem = static_cast<int>(index);
        item.pszText = const_cast<LPWSTR>(entry.name.c_str());
        item.iImage = image;
        item.lParam = static_cast<LPARAM>(index);
        const int list_index = ListView_InsertItem(list, &item);
        if (list_index < 0) check(E_FAIL);
        if (state.entries[index].id == selected_id)
        {
            ListView_SetItemState(list, list_index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
            ListView_EnsureVisible(list, list_index, FALSE);
        }
    }
    state.loading = false;
}

void load_selected_entry(HWND dialog, DialogState& state)
{
    if (state.loading) return;
    const HWND list = GetDlgItem(dialog, IDC_THIS_PC_LIST);
    const int selected = ListView_GetNextItem(list, -1, LVNI_SELECTED);
    if (selected < 0)
    {
        clear_editor(dialog, state);
        return;
    }
    LVITEMW item{};
    item.mask = LVIF_PARAM;
    item.iItem = selected;
    check_win32(ListView_GetItem(list, &item));
    const size_t index = static_cast<size_t>(item.lParam);
    if (index >= state.entries.size()) check(E_FAIL);
    const auto& entry = state.entries[index];
    state.loading = true;
    state.selected_id = entry.id;
    set_control_text(dialog, IDC_THIS_PC_NAME, entry.name);
    set_control_text(dialog, IDC_THIS_PC_TARGET, entry.target);
    set_control_text(dialog, IDC_THIS_PC_ICON, entry.icon);
    state.loading = false;
    update_editor(dialog, state);
    update_preview(dialog, state);
}

void choose_new_folder(HWND dialog, DialogState& state)
{
    const auto selected = choose_folder(dialog);
    if (!selected) return;
    // Choosing another target while editing keeps the selected entry's identity.
    if (state.selected_id.empty()) set_control_text(dialog, IDC_THIS_PC_NAME, selected->second);
    set_control_text(dialog, IDC_THIS_PC_TARGET, selected->first);
    update_editor(dialog, state);
    SetFocus(GetDlgItem(dialog, IDC_THIS_PC_NAME));
}

void choose_entry_icon(HWND dialog, DialogState& state)
{
    const auto icon = choose_icon(dialog);
    if (icon) set_control_text(dialog, IDC_THIS_PC_ICON, *icon);
    update_preview(dialog, state);
}

void save_editor(HWND dialog, DialogState& state)
{
    ThisPcEntry entry;
    entry.id = state.selected_id.empty() ? new_entry_id() : state.selected_id;
    entry.name = trim(dialog_text(dialog, IDC_THIS_PC_NAME));
    entry.target = trim(dialog_text(dialog, IDC_THIS_PC_TARGET));
    entry.icon = trim(dialog_text(dialog, IDC_THIS_PC_ICON));
    if (entry.name.empty() || entry.target.empty() || entry.icon.empty()) check(E_INVALIDARG);

    const DWORD attributes = GetFileAttributesW(entry.target.c_str());
    if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY)) check(E_INVALIDARG);

    const auto existing = std::find_if(state.entries.begin(), state.entries.end(), [&](const ThisPcEntry& candidate) {
        return candidate.id == entry.id;
    });
    entry.sort_order = existing == state.entries.end()
        ? 80 + static_cast<DWORD>(state.entries.size())
        : existing->sort_order;
    if (is_elevated()) write_entry(entry);
    else if (!run_elevated_worker(dialog, {L"save", entry.id, entry.name, entry.target, entry.icon})) return;
    notify_this_pc();
    reload_entries(dialog, state, entry.id);
    load_selected_entry(dialog, state);
}

void remove_selected_entry(HWND dialog, DialogState& state)
{
    if (state.selected_id.empty()) check(E_INVALIDARG);
    const auto existing = std::find_if(state.entries.begin(), state.entries.end(), [&](const ThisPcEntry& candidate) {
        return candidate.id == state.selected_id;
    });
    if (existing == state.entries.end()) check(E_FAIL);
    if (is_elevated()) remove_entry(*existing);
    else if (!run_elevated_worker(dialog, {L"remove", existing->id})) return;
    notify_this_pc();
    reload_entries(dialog, state);
    clear_editor(dialog, state);
}

INT_PTR CALLBACK this_pc_dialog(HWND dialog, UINT message, WPARAM first, LPARAM second) noexcept
{
    if (message == WM_INITDIALOG)
    {
        const auto status = guarded([&] {
            auto* state = reinterpret_cast<DialogState*>(second);
            SetWindowLongPtrW(dialog, DWLP_USER, second);
            SendDlgItemMessageW(dialog, IDC_THIS_PC_NAME, EM_SETLIMITTEXT, 120, 0);
            reload_entries(dialog, *state);
            clear_editor(dialog, *state);
            return S_OK;
        });
        if (FAILED(status))
        {
            fail_native_dialog(dialog, status);
        }
        return TRUE;
    }

    auto* state = reinterpret_cast<DialogState*>(GetWindowLongPtrW(dialog, DWLP_USER));
    if (!state) return FALSE;
    if (message == WM_NOTIFY)
    {
        const auto* notice = reinterpret_cast<NMHDR*>(second);
        if (notice->idFrom != IDC_THIS_PC_LIST || notice->code != LVN_ITEMCHANGED) return FALSE;
        const auto* change = reinterpret_cast<NMLISTVIEW*>(second);
        if (!(change->uChanged & LVIF_STATE) || !(change->uNewState & LVIS_SELECTED)) return FALSE;
        const auto status = guarded([&] { load_selected_entry(dialog, *state); return S_OK; });
        if (FAILED(status)) report_error(dialog, status, IDS_ERROR_THIS_PC);
        return TRUE;
    }
    if (message == WM_APP + 19)
    {
        const auto status = guarded([&] {
            const auto selected = state->selected_id;
            reload_entries(dialog, *state, selected);
            update_preview(dialog, *state);
            return S_OK;
        });
        if (FAILED(status)) report_error(dialog, status, IDS_ERROR_THIS_PC);
        return TRUE;
    }
    if (message != WM_COMMAND) return FALSE;

    const auto status = guarded([&] {
        switch (LOWORD(first))
        {
        case IDC_THIS_PC_NAME:
            if (HIWORD(first) == EN_CHANGE) update_editor(dialog, *state);
            return S_OK;
        case IDC_THIS_PC_NEW:
            clear_editor(dialog, *state);
            choose_new_folder(dialog, *state);
            return S_OK;
        case IDC_THIS_PC_CHOOSE_FOLDER:
            choose_new_folder(dialog, *state);
            return S_OK;
        case IDC_THIS_PC_CHOOSE_ICON:
            choose_entry_icon(dialog, *state);
            return S_OK;
        case IDC_THIS_PC_SAVE:
            save_editor(dialog, *state);
            return S_OK;
        case IDC_THIS_PC_REMOVE:
            remove_selected_entry(dialog, *state);
            return S_OK;
        default:
            return S_FALSE;
        }
    });
    if (FAILED(status)) report_error(dialog, status, IDS_ERROR_THIS_PC);
    return TRUE;
}
}


std::unique_ptr<NativeDialogPage> create_this_pc_page(HWND parent)
{
    auto state = std::make_shared<DialogState>();
    const DialogAnchor anchors[]{
        {IDC_DIALOG_HEADING, true}, {IDC_THIS_PC_DESCRIPTION, true}, {IDC_DIALOG_HEADER_LINE, true},
        {IDC_THIS_PC_LIST, false, false, true}, {IDC_THIS_PC_MODE, true},
        {IDC_THIS_PC_NAME, true}, {IDC_THIS_PC_TARGET, true},
        {IDC_THIS_PC_CHOOSE_FOLDER, false, true},
        {IDC_THIS_PC_ICON, true}, {IDC_THIS_PC_CHOOSE_ICON, false, true},
        {IDC_DIALOG_FOOTER_LINE, true, false, false, true},
        {IDC_THIS_PC_NEW, false, false, false, true}, {IDC_THIS_PC_REMOVE, false, false, false, true},
        {IDC_THIS_PC_SAVE, false, true, false, true}};
    return NativeDialogPage::create(IDD_THIS_PC_ENTRIES, parent, this_pc_dialog,
        reinterpret_cast<LPARAM>(state.get()), anchors, state);
}

HRESULT run_this_pc_worker(const std::vector<std::wstring>& arguments)
{
    return guarded([&] {
        if (!is_elevated() || arguments.size() < 3 || arguments[0] != current_user_sid()) return E_ACCESSDENIED;
        const auto& id = arguments[2];
        GUID parsed{};
        if (id.size() != 38 || id.front() != L'{' || id.back() != L'}' || FAILED(CLSIDFromString(id.c_str(), &parsed)))
            return E_INVALIDARG;
        auto entries = read_entries();
        const auto existing = std::find_if(entries.begin(), entries.end(), [&](const ThisPcEntry& candidate) {
            return _wcsicmp(candidate.id.c_str(), id.c_str()) == 0;
        });
        if (arguments[1] == L"remove" && arguments.size() == 3)
        {
            if (existing == entries.end()) return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
            remove_entry(*existing);
        }
        else if (arguments[1] == L"save" && arguments.size() == 6)
        {
            ThisPcEntry entry{id, trim(arguments[3]), trim(arguments[4]), trim(arguments[5]),
                existing == entries.end() ? 80 + static_cast<DWORD>(entries.size()) : existing->sort_order};
            if (entry.name.empty() || entry.name.size() > 120 || entry.target.empty() || entry.icon.empty()) return E_INVALIDARG;
            const DWORD attributes = GetFileAttributesW(entry.target.c_str());
            if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY)) return E_INVALIDARG;
            write_entry(entry);
        }
        else return E_INVALIDARG;
        notify_this_pc();
        return S_OK;
    });
}

HRESULT add_this_pc_folders(const std::vector<std::wstring>& paths)
{
    return guarded([&] {
        if (paths.empty()) return E_INVALIDARG;
        if (!is_elevated())
        {
            std::wstring parameters{L"--add-this-pc"};
            for (const auto& path : paths)
            {
                parameters += L" ";
                parameters += quote_command_line_argument(path);
            }
            return launch_elevated(parameters);
        }

        auto entries = read_entries();
        for (const auto& original_path : paths)
        {
            const auto target = std::filesystem::path(original_path).lexically_normal().wstring();
            const DWORD attributes = GetFileAttributesW(target.c_str());
            if (attributes == INVALID_FILE_ATTRIBUTES || !(attributes & FILE_ATTRIBUTE_DIRECTORY)) return E_INVALIDARG;
            const auto already_added = std::find_if(entries.begin(), entries.end(), [&](const ThisPcEntry& entry) {
                return _wcsicmp(entry.target.c_str(), target.c_str()) == 0;
            });
            if (already_added != entries.end()) continue;

            const auto pidl = parse_pidl(target);
            const auto item = shell_item(pidl.get());
            ThisPcEntry entry{
                new_entry_id(),
                display_name(item.Get()),
                target,
                L"%SystemRoot%\\System32\\imageres.dll,-3",
                80 + static_cast<DWORD>(entries.size())};
            write_entry(entry);
            entries.push_back(std::move(entry));
        }
        notify_this_pc();
        return S_OK;
    });
}
}
