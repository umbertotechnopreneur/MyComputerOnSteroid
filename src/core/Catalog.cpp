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

#include "Catalog.hpp"
#include "Recents.hpp"
#include "../../resources/resource.h"
#include <algorithm>
#include <cwctype>
#include <array>

namespace mcos
{
namespace
{
bool is_link(const std::filesystem::path& path)
{
    return _wcsicmp(path.extension().c_str(), L".lnk") == 0;
}

ComPtr<IShellLinkW> load_link(const std::filesystem::path& path)
{
    ComPtr<IShellLinkW> link;
    check(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&link)));
    ComPtr<IPersistFile> file; check(link.As(&file));
    check(file->Load(path.c_str(), STGM_READ));
    return link;
}

std::wstring safe_name(std::wstring name)
{
    for (auto& character : name)
        if (character < 32 || wcschr(L"<>:\"/\\|?*", character)) character = L'_';
    while (!name.empty() && (name.back() == L'.' || name.back() == L' ')) name.pop_back();
    if (name.empty()) name = L"Favorite";
    if (name.size() > 120) name.resize(120);
    return name;
}

void validate_store(Section section)
{
    const auto root = data_root();
    for (const auto& path : {root, root / L"Favorites", section_directory(section)})
    {
        const auto flags = GetFileAttributesW(path.c_str());
        if (flags == INVALID_FILE_ATTRIBUTES)
        {
            const auto error = GetLastError();
            if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND) continue;
            check(HRESULT_FROM_WIN32(error));
        }
        if (!(flags & FILE_ATTRIBUTE_DIRECTORY) || flags & FILE_ATTRIBUTE_REPARSE_POINT)
            check(E_ACCESSDENIED);
    }
}
}

std::filesystem::path data_root()
{
    // Test hosts opt into an isolated store; Explorer normally has no override.
    const DWORD size = GetEnvironmentVariableW(L"MCOS_DATA_DIR", nullptr, 0);
    if (size)
    {
        std::wstring value(size, L'\0');
        const DWORD read = GetEnvironmentVariableW(L"MCOS_DATA_DIR", value.data(), size);
        if (!read || read >= size) check(E_FAIL);
        value.resize(read);
        auto root = std::filesystem::path(value).lexically_normal();
        if (!root.is_absolute()) check(E_INVALIDARG);
        return root;
    }
    PWSTR raw = nullptr;
    check(SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_DEFAULT, nullptr, &raw));
    TaskString owned(raw);
    return std::filesystem::path(raw) / L"MyComputerOnSteroid";
}

std::filesystem::path section_directory(Section section)
{
    const wchar_t* folder = nullptr;
    switch (section)
    {
    case Section::folders: folder = L"Folders"; break;
    case Section::apps: folder = L"Apps"; break;
    case Section::files: folder = L"Files"; break;
    default: check(E_INVALIDARG);
    }
    return data_root() / L"Favorites" / folder;
}

std::vector<CatalogItem> enumerate_catalog(bool include_recents)
{
    std::vector<CatalogItem> items;
    wchar_t drives[512]{};
    const DWORD length = GetLogicalDriveStringsW(ARRAYSIZE(drives), drives);
    if (!length || length >= ARRAYSIZE(drives)) check(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
    for (const wchar_t* drive = drives; *drive; drive += wcslen(drive) + 1)
        items.push_back({Section::drives, drive});

    for (auto section : {Section::folders, Section::apps, Section::files})
    {
        const auto directory = section_directory(section);
        validate_store(section);
        if (!std::filesystem::exists(directory)) continue;
        // Never interpret a junction under the store as owned favorite storage.
        if (GetFileAttributesW(directory.c_str()) & FILE_ATTRIBUTE_REPARSE_POINT) check(E_ACCESSDENIED);
        for (const auto& entry : std::filesystem::directory_iterator(directory))
        {
            const auto path = entry.path();
            const auto flags = GetFileAttributesW(path.c_str());
            if (flags == INVALID_FILE_ATTRIBUTES || flags & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))
                continue;
            if (is_link(path) && !path.filename().wstring().starts_with(L".mcos-"))
                items.push_back({section, path.wstring()});
        }
    }
    if (include_recents)
    {
        const auto status = guarded([&] {
            auto recent = enumerate_recents();
            items.insert(items.end(), std::make_move_iterator(recent.begin()), std::make_move_iterator(recent.end()));
            return S_OK;
        });
        // The optional history source must never blank drives or saved favorites.
        if (FAILED(status)) report_error(nullptr, status, IDS_ERROR_RECENTS);
    }
    return items;
}

Section classify(IShellItem* target)
{
    SFGAOF attributes = 0;
    check(target->GetAttributes(SFGAO_FOLDER | SFGAO_FILESYSTEM, &attributes));
    if (!(attributes & SFGAO_FILESYSTEM)) return Section::apps;
    if (attributes & SFGAO_FOLDER) return Section::folders;
    const auto extension = std::filesystem::path(display_name(target, SIGDN_FILESYSPATH)).extension().wstring();
    for (const auto* app : {L".exe", L".com", L".cmd", L".bat", L".ps1"})
        if (_wcsicmp(extension.c_str(), app) == 0) return Section::apps;
    return Section::files;
}

std::filesystem::path add_favorite(IShellItem* target, Section requested, const std::wstring& preferred_name)
{
    if (!target) check(E_POINTER);
    PIDLIST_ABSOLUTE target_raw = nullptr;
    check(SHGetIDListFromObject(target, &target_raw));
    Pidl target_id(target_raw);
    ComPtr<IShellLinkW> link;
    std::wstring name = preferred_name.empty() ? display_name(target) : preferred_name;
    SFGAOF attributes = 0;
    check(target->GetAttributes(SFGAO_FILESYSTEM | SFGAO_LINK, &attributes));
    if ((attributes & (SFGAO_FILESYSTEM | SFGAO_LINK)) == (SFGAO_FILESYSTEM | SFGAO_LINK) &&
        is_link(display_name(target, SIGDN_FILESYSPATH)))
    {
        link = load_link(display_name(target, SIGDN_FILESYSPATH));
        PIDLIST_ABSOLUTE resolved = nullptr;
        check(link->GetIDList(&resolved));
        if (!resolved) check(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
        target_id.reset(resolved);
    }
    else
    {
        check(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&link)));
        check(link->SetIDList(target_id.get()));
    }
    const Section section = requested == static_cast<Section>(0) ? classify(shell_item(target_id.get()).Get()) : requested;
    if (!is_favorite_section(section)) check(E_INVALIDARG);
    const auto directory = section_directory(section);
    validate_store(section);
    std::filesystem::create_directories(directory);
    validate_store(section);
    if (GetFileAttributesW(directory.c_str()) & FILE_ATTRIBUTE_REPARSE_POINT) check(E_ACCESSDENIED);

    std::array<wchar_t, 32768> arguments{};
    check(link->GetArguments(arguments.data(), static_cast<int>(arguments.size())));
    for (const auto& existing : enumerate_catalog(false))
    {
        if (existing.section != section) continue;
        // Broken links should not prevent adding an unrelated favorite.
        bool duplicate = false;
        guarded([&] {
            auto candidate = load_link(existing.path);
            PIDLIST_ABSOLUTE raw = nullptr; check(candidate->GetIDList(&raw)); Pidl id(raw);
            std::array<wchar_t, 32768> other_arguments{};
            check(candidate->GetArguments(other_arguments.data(), static_cast<int>(other_arguments.size())));
            duplicate = id && ILIsEqual(id.get(), target_id.get()) && wcscmp(arguments.data(), other_arguments.data()) == 0;
            return S_OK;
        });
        if (duplicate) return existing.path;
    }

    if (is_link(name)) name = std::filesystem::path(name).stem().wstring();
    name = safe_name(name);
    GUID guid{}; check(CoCreateGuid(&guid));
    wchar_t guid_text[40]{}; StringFromGUID2(guid, guid_text, ARRAYSIZE(guid_text));
    const auto temporary = directory / (std::wstring(L".mcos-") + guid_text + L".lnk");
    ComPtr<IPersistFile> file; check(link.As(&file));
    check(file->Save(temporary.c_str(), TRUE));
    try
    {
        for (unsigned suffix = 1; suffix < 10000; ++suffix)
        {
            const auto final = directory / (name + (suffix == 1 ? L"" : L" (" + std::to_wstring(suffix) + L")") + L".lnk");
            if (MoveFileExW(temporary.c_str(), final.c_str(), MOVEFILE_WRITE_THROUGH))
            {
                notify_launcher();
                return final;
            }
            const auto error = GetLastError();
            if (error != ERROR_ALREADY_EXISTS && error != ERROR_FILE_EXISTS) check(HRESULT_FROM_WIN32(error));
        }
        check(HRESULT_FROM_WIN32(ERROR_FILE_EXISTS));
    }
    catch (...) { DeleteFileW(temporary.c_str()); throw; }
    check(E_FAIL);
    return {};
}

bool is_owned_favorite(const CatalogItem& item)
{
    if (!is_favorite_section(item.section)) return false;
    if (FAILED(guarded([&] { validate_store(item.section); return S_OK; }))) return false;
    const auto path = std::filesystem::path(item.path).lexically_normal();
    const auto expected = section_directory(item.section).lexically_normal();
    if (!path.is_absolute() || !is_link(path) || _wcsicmp(path.parent_path().c_str(), expected.c_str()) != 0) return false;
    const auto directory_flags = GetFileAttributesW(expected.c_str());
    const auto flags = GetFileAttributesW(path.c_str());
    return directory_flags != INVALID_FILE_ATTRIBUTES && !(directory_flags & FILE_ATTRIBUTE_REPARSE_POINT) &&
        flags != INVALID_FILE_ATTRIBUTES && !(flags & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT));
}

void remove_favorite(const CatalogItem& item)
{
    if (!is_owned_favorite(item)) check(E_ACCESSDENIED);
    check_win32(DeleteFileW(item.path.c_str())); // Deletes only our .lnk, never resolves its target.
    notify_launcher();
}

CatalogItem rename_favorite(const CatalogItem& item, const std::wstring& name)
{
    if (!is_owned_favorite(item)) check(E_ACCESSDENIED);
    if (name.empty() || safe_name(name) != name) check(E_INVALIDARG);
    std::wstring filename = name;
    if (!is_link(filename)) filename += L".lnk";
    const auto destination = section_directory(item.section) / filename;
    if (item.path != destination.wstring())
        check_win32(MoveFileExW(item.path.c_str(), destination.c_str(), MOVEFILE_WRITE_THROUGH));
    notify_launcher();
    return {item.section, destination.wstring()};
}

void change_favorite_icon(const CatalogItem& item, const std::filesystem::path& icon)
{
    if (!is_owned_favorite(item)) check(E_ACCESSDENIED);
    if (!icon.is_absolute() || _wcsicmp(icon.extension().c_str(), L".ico") != 0) check(E_INVALIDARG);
    // Accept an actual ICO, rather than trusting the extension. Do not change the target.
    const auto preview = static_cast<HICON>(LoadImageW(nullptr, icon.c_str(), IMAGE_ICON, 0, 0,
        LR_LOADFROMFILE | LR_DEFAULTSIZE));
    if (!preview) check(HRESULT_FROM_WIN32(ERROR_INVALID_DATA));
    DestroyIcon(preview);
    auto link = load_link(item.path);
    check(link->SetIconLocation(icon.c_str(), 0));
    GUID id{}; check(CoCreateGuid(&id));
    wchar_t value[40]{}; StringFromGUID2(id, value, ARRAYSIZE(value));
    const auto temporary = section_directory(item.section) / (std::wstring(L".mcos-") + value + L".lnk");
    ComPtr<IPersistFile> file; check(link.As(&file));
    try
    {
        check(file->Save(temporary.c_str(), TRUE));
        if (!is_owned_favorite(item)) check(E_ACCESSDENIED);
        check_win32(MoveFileExW(temporary.c_str(), item.path.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH));
    }
    catch (...) { DeleteFileW(temporary.c_str()); throw; }
    SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATHW | SHCNF_FLUSHNOWAIT, item.path.c_str(), nullptr);
    notify_launcher();
}

void choose_favorite_icon(HWND owner, const std::vector<CatalogItem>& items)
{
    if (items.empty()) check(E_INVALIDARG);
    for (const auto& item : items) if (!is_owned_favorite(item)) check(E_ACCESSDENIED);
    ComPtr<IFileOpenDialog> dialog;
    check(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)));
    // A separate client GUID gives the icon picker its own Windows-maintained last folder.
    constexpr GUID picker{0xb36b9212, 0x2d35, 0x4e65, {0x8a, 0x97, 0x0a, 0xa5, 0xd2, 0x80, 0xe1, 0x61}};
    check(dialog->SetClientGuid(picker));
    FILEOPENDIALOGOPTIONS options{}; check(dialog->GetOptions(&options));
    check(dialog->SetOptions(options | FOS_FORCEFILESYSTEM | FOS_FILEMUSTEXIST | FOS_NOCHANGEDIR | FOS_DONTADDTORECENT));
    const auto label = text(IDS_ICON_FILES);
    const COMDLG_FILTERSPEC filter[]{ {label.c_str(), L"*.ico"} };
    check(dialog->SetFileTypes(ARRAYSIZE(filter), filter));
    check(dialog->SetTitle(text(IDS_CHANGE_ICON).c_str()));
    const auto status = dialog->Show(owner);
    if (status == HRESULT_FROM_WIN32(ERROR_CANCELLED)) return;
    check(status);
    ComPtr<IShellItem> selected; check(dialog->GetResult(&selected));
    const auto path = std::filesystem::path(display_name(selected.Get(), SIGDN_FILESYSPATH));
    for (const auto& item : items) change_favorite_icon(item, path);
}

Pidl navigation_target(const CatalogItem& item)
{
    if (item.section == Section::drives) return parse_pidl(item.path);
    auto link = load_link(item.path);
    PIDLIST_ABSOLUTE raw = nullptr;
    check(link->GetIDList(&raw));
    if (!raw) check(HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND));
    return Pidl(raw);
}

std::wstring item_name(const CatalogItem& item)
{
    if (item.section == Section::recents) return recent_info(item).name;
    if (item.section == Section::drives) return display_name(shell_item(parse_pidl(item.path).get()).Get());
    return std::filesystem::path(item.path).stem().wstring();
}

void choose_favorite(HWND owner, Section section)
{
    ComPtr<IFileOpenDialog> dialog;
    check(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog)));
    FILEOPENDIALOGOPTIONS options{};
    check(dialog->GetOptions(&options));
    options |= FOS_ALLOWMULTISELECT | FOS_FILEMUSTEXIST | FOS_NOCHANGEDIR;
    if (section == Section::folders) options |= FOS_PICKFOLDERS;
    check(dialog->SetOptions(options));
    const auto title = text(section == Section::folders ? IDS_ADD_FOLDER :
        section == Section::apps ? IDS_ADD_APP : IDS_ADD_FILE);
    check(dialog->SetTitle(title.c_str()));
    const HRESULT shown = dialog->Show(owner);
    if (shown == HRESULT_FROM_WIN32(ERROR_CANCELLED)) return;
    check(shown);
    ComPtr<IShellItemArray> items; check(dialog->GetResults(&items));
    DWORD count{}; check(items->GetCount(&count));
    for (DWORD index = 0; index < count; ++index)
    {
        ComPtr<IShellItem> item; check(items->GetItemAt(index, &item));
        add_favorite(item.Get(), section);
    }
}
}
