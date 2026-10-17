// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; isolated recent-item cache and filters; 2026-10-11.
 * Human guidance: Umberto Giacobbi; do not delete originals or Windows history.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#include "Recents.hpp"
#include "../../resources/resource.h"
#include <bcrypt.h>
#include <algorithm>
#include <array>
#include <cwctype>
#include <cstring>
#include <set>

namespace mcos
{
namespace
{
constexpr size_t recent_limit = 512;
std::filesystem::path recent_store() { return data_root() / L"Recents"; }
std::filesystem::path cache_directory() { return recent_store() / L"Items"; }
std::filesystem::path settings_file() { return recent_store() / L"settings.ini"; }

DWORD attributes(const std::filesystem::path& path)
{
    const auto flags = GetFileAttributesW(path.c_str());
    if (flags == INVALID_FILE_ATTRIBUTES)
    {
        const auto error = GetLastError();
        if (error != ERROR_FILE_NOT_FOUND && error != ERROR_PATH_NOT_FOUND) check(HRESULT_FROM_WIN32(error));
    }
    return flags;
}
void validate_directories()
{
    for (const auto& path : {data_root(), recent_store(), cache_directory()})
    {
        const auto flags = attributes(path);
        if (flags != INVALID_FILE_ATTRIBUTES && (!(flags & FILE_ATTRIBUTE_DIRECTORY) || flags & FILE_ATTRIBUTE_REPARSE_POINT))
            check(E_ACCESSDENIED);
    }
}
void validate_file(const std::filesystem::path& path)
{
    const auto flags = attributes(path);
    if (flags != INVALID_FILE_ATTRIBUTES && flags & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT))
        check(E_ACCESSDENIED);
}
std::wstring lower(std::wstring value)
{
    std::transform(value.begin(), value.end(), value.begin(), [](wchar_t c) { return static_cast<wchar_t>(towlower(c)); });
    return value;
}
std::wstring digest(const std::wstring& value)
{
    std::array<UCHAR, 32> hash{};
    if (BCryptHash(BCRYPT_SHA256_ALG_HANDLE, nullptr, 0,
        reinterpret_cast<PUCHAR>(const_cast<wchar_t*>(value.data())), static_cast<ULONG>(value.size() * sizeof(wchar_t)),
        hash.data(), static_cast<ULONG>(hash.size())) < 0) check(E_FAIL);
    constexpr wchar_t digits[] = L"0123456789abcdef";
    std::wstring result;
    for (const auto byte : hash) { result.push_back(digits[byte >> 4]); result.push_back(digits[byte & 15]); }
    return result;
}
bool valid_key(const std::wstring& key)
{
    return key.size() == 64 && std::all_of(key.begin(), key.end(), [](wchar_t c) {
        return (c >= L'0' && c <= L'9') || (c >= L'a' && c <= L'f');
    });
}
std::filesystem::path record_file(const std::filesystem::path& link)
{
    return link.parent_path() / (link.stem().wstring() + L".ini");
}
std::wstring read(const std::filesystem::path& file, PCWSTR key, PCWSTR fallback = L"", PCWSTR section = L"Entry")
{
    validate_directories(); validate_file(file);
    std::array<wchar_t, 1024> value{};
    const auto length = GetPrivateProfileStringW(section, key, fallback, value.data(), static_cast<DWORD>(value.size()), file.c_str());
    if (length >= value.size() - 1) check(HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER));
    return value.data();
}
void write(const std::filesystem::path& file, PCWSTR key, const std::wstring& value, PCWSTR section = L"Entry")
{
    validate_directories(); validate_file(file);
    std::filesystem::create_directories(file.parent_path());
    validate_directories();
    // An explicit UTF-16 BOM keeps Windows profile APIs lossless for Unicode filters/names.
    const HANDLE created = CreateFileW(file.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (created != INVALID_HANDLE_VALUE)
    {
        const WORD bom = 0xfeff; DWORD written{};
        const BOOL saved = WriteFile(created, &bom, sizeof(bom), &written, nullptr);
        const auto error = GetLastError(); CloseHandle(created);
        if (!saved) check(HRESULT_FROM_WIN32(error));
        if (written != sizeof(bom)) check(E_FAIL);
    }
    else if (GetLastError() != ERROR_FILE_EXISTS) check(HRESULT_FROM_WIN32(GetLastError()));
    check_win32(WritePrivateProfileStringW(section, key, value.c_str(), file.c_str()));
}
ULONGLONG integer(const std::wstring& value)
{
    if (value.empty()) return 0;
    if (!std::all_of(value.begin(), value.end(), [](wchar_t c) { return c >= L'0' && c <= L'9'; })) check(E_INVALIDARG);
    return std::stoull(value);
}
ULONGLONG ticks(FILETIME value)
{ return (static_cast<ULONGLONG>(value.dwHighDateTime) << 32) | value.dwLowDateTime; }
FILETIME filetime(ULONGLONG value)
{ return {static_cast<DWORD>(value), static_cast<DWORD>(value >> 32)}; }

// Synchronize cache refresh with mutations from another Explorer window. No worker
// thread or initialization runs in DllMain; the bounded lock belongs to the operation.
class RecentLock
{
public:
    explicit RecentLock(bool try_only = false)
    {
        const auto name = L"Local\\MCOS.Recents." + digest(lower(data_root().wstring()));
        handle_ = CreateMutexW(nullptr, FALSE, name.c_str());
        if (!handle_) check(HRESULT_FROM_WIN32(GetLastError()));
        const auto result = WaitForSingleObject(handle_, try_only ? 0 : 2000);
        if (result != WAIT_OBJECT_0 && result != WAIT_ABANDONED)
        {
            CloseHandle(handle_); handle_ = nullptr;
            if (result != WAIT_TIMEOUT || !try_only) check(HRESULT_FROM_WIN32(ERROR_BUSY));
        }
    }
    ~RecentLock() { if (handle_) { ReleaseMutex(handle_); CloseHandle(handle_); } }
    RecentLock(const RecentLock&) = delete;
    RecentLock& operator=(const RecentLock&) = delete;
    bool acquired() const noexcept { return handle_ != nullptr; }
private:
    HANDLE handle_{nullptr};
};
ComPtr<IShellLinkW> load(const std::filesystem::path& path)
{
    ComPtr<IShellLinkW> link;
    check(CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&link)));
    ComPtr<IPersistFile> file; check(link.As(&file)); check(file->Load(path.c_str(), STGM_READ));
    return link; // Deliberately never call Resolve (no target search, repair or UI).
}
std::filesystem::path windows_recents()
{
    const DWORD size = GetEnvironmentVariableW(L"MCOS_RECENT_DIR", nullptr, 0);
    if (size)
    {
        if (!GetEnvironmentVariableW(L"MCOS_DATA_DIR", nullptr, 0)) check(E_ACCESSDENIED);
        std::wstring value(size, L'\0');
        const auto length = GetEnvironmentVariableW(L"MCOS_RECENT_DIR", value.data(), size);
        if (!length || length >= size) check(E_INVALIDARG);
        value.resize(length);
        std::filesystem::path path(value);
        if (!path.is_absolute()) check(E_INVALIDARG);
        return path.lexically_normal();
    }
    PWSTR raw = nullptr; check(SHGetKnownFolderPath(FOLDERID_Recent, KF_FLAG_DEFAULT, nullptr, &raw));
    TaskString owned(raw); return std::filesystem::path(raw);
}
std::wstring target_path(IShellLinkW* link)
{
    std::array<wchar_t, 32768> raw{};
    check(link->GetPath(raw.data(), static_cast<int>(raw.size()), nullptr, SLGP_RAWPATH));
    // This source is for recent filesystem documents. Binding a virtual PIDL here
    // can re-enter a namespace provider (including this one) during enumeration.
    // Do not bind Shell targets, contact servers or repair a shortcut just to index it.
    if (!raw[0] || !std::filesystem::path(raw.data()).is_absolute()) check(E_INVALIDARG);
    return raw.data();
}
void save_link(IShellLinkW* link, const std::filesystem::path& destination)
{
    validate_directories(); validate_file(destination);
    std::filesystem::create_directories(cache_directory()); validate_directories();
    GUID guid{}; check(CoCreateGuid(&guid));
    wchar_t value[40]{}; StringFromGUID2(guid, value, ARRAYSIZE(value));
    const auto temporary = cache_directory() / (std::wstring(L".mcos-") + value + L".lnk");
    ComPtr<IPersistFile> file; check(link->QueryInterface(IID_PPV_ARGS(&file)));
    try
    {
        check(file->Save(temporary.c_str(), TRUE));
        validate_file(destination);
        check_win32(MoveFileExW(temporary.c_str(), destination.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH));
    }
    catch (...) { DeleteFileW(temporary.c_str()); throw; }
}
void validate_selection(const std::vector<CatalogItem>& items)
{
    if (items.empty()) check(E_INVALIDARG);
    for (const auto& item : items) if (!is_owned_recent(item)) check(E_ACCESSDENIED);
}
std::vector<CatalogItem> cached_items(const RecentSettings& settings)
{
    struct Cached { CatalogItem item; RecentInfo info; };
    std::vector<Cached> cached;
    validate_directories();
    if (!std::filesystem::exists(cache_directory())) return {};
    const auto name_filter = lower(settings.name);
    for (const auto& entry : std::filesystem::directory_iterator(cache_directory()))
    {
        const auto path = entry.path();
        CatalogItem item{Section::recents, path.wstring()};
        if (_wcsicmp(path.extension().c_str(), L".lnk") || !is_owned_recent(item)) continue;
        const auto record = record_file(path);
        const auto info = recent_info(item);
        if (read(record, L"Hidden", L"0") == L"1" || (!info.pinned && read(record, L"Present", L"0") != L"1")) continue;
        const auto extension = read(record, L"Extension");
        if (settings.category != RecentCategory::all && recent_category(extension) != settings.category) continue;
        if (!settings.extension.empty() && _wcsicmp(settings.extension.c_str(), extension.c_str())) continue;
        if (lower(info.name).find(name_filter) == std::wstring::npos) continue;
        cached.push_back({std::move(item), info});
    }
    // Read metadata once per item, rather than making disk calls in O(n log n)
    // comparisons while another Explorer window is waiting for this cache.
    std::sort(cached.begin(), cached.end(), [](const Cached& a, const Cached& b) {
        if (ticks(a.info.updated) != ticks(b.info.updated)) return ticks(a.info.updated) > ticks(b.info.updated);
        return StrCmpLogicalW(a.info.name.c_str(), b.info.name.c_str()) < 0;
    });
    std::vector<CatalogItem> result;
    for (auto& entry : cached) result.push_back(std::move(entry.item));
    return result;
}
}

RecentSettings recent_settings()
{
    RecentSettings result;
    const auto path = settings_file();
    result.enabled = read(path, L"Enabled", L"0", L"Settings") == L"1";
    result.name = read(path, L"Name", L"", L"Settings");
    const auto category = integer(read(path, L"Category", L"0", L"Settings"));
    if (category > static_cast<unsigned>(RecentCategory::other)) check(E_INVALIDARG);
    result.category = static_cast<RecentCategory>(category);
    result.extension = read(path, L"Extension", L"", L"Settings");
    return result;
}
void validate_recent_settings(const RecentSettings& settings)
{
    if (settings.name.size() > 256 || settings.name.find_first_of(L"\r\n") != std::wstring::npos ||
        settings.extension.size() > 32 || settings.extension.find_first_of(L"\r\n/\\*?;:") != std::wstring::npos ||
        static_cast<unsigned>(settings.category) > static_cast<unsigned>(RecentCategory::other)) check(E_INVALIDARG);
}

void save_recent_settings(const RecentSettings& settings)
{
    validate_recent_settings(settings);
    {
        RecentLock lock;
        auto extension = lower(settings.extension);
        if (!extension.empty() && extension[0] != L'.') extension.insert(extension.begin(), L'.');
        const auto path = settings_file();
        write(path, L"Enabled", settings.enabled ? L"1" : L"0", L"Settings");
        write(path, L"Name", settings.name, L"Settings");
        write(path, L"Category", std::to_wstring(static_cast<unsigned>(settings.category)), L"Settings");
        write(path, L"Extension", extension, L"Settings");
    } // Notify only after releasing the lock: Shell parsing can re-enter enumeration.
    notify_launcher();
}

RecentCategory recent_category(const std::wstring& extension)
{
    const auto value = lower(extension);
    for (PCWSTR kind : {L".pdf", L".doc", L".docx", L".docm", L".xls", L".xlsx", L".xlsm", L".ppt", L".pptx", L".odt", L".ods", L".odp", L".rtf", L".txt", L".md", L".csv", L".epub"})
        if (value == kind) return RecentCategory::documents;
    for (PCWSTR kind : {L".png", L".jpg", L".jpeg", L".gif", L".bmp", L".webp", L".heic", L".heif", L".avif", L".tif", L".tiff", L".svg", L".ico", L".raw"})
        if (value == kind) return RecentCategory::images;
    for (PCWSTR kind : {L".mp3", L".wav", L".flac", L".aac", L".m4a", L".ogg", L".opus", L".wma", L".aiff"})
        if (value == kind) return RecentCategory::audio;
    for (PCWSTR kind : {L".mp4", L".mkv", L".mov", L".avi", L".webm", L".wmv", L".m4v", L".mpeg", L".mpg"})
        if (value == kind) return RecentCategory::video;
    for (PCWSTR kind : {L".css", L".html", L".htm", L".js", L".jsx", L".ts", L".tsx", L".vue", L".json", L".xml", L".yaml", L".yml", L".c", L".cpp", L".h", L".hpp", L".cs", L".py", L".ps1", L".sh", L".sql", L".rs", L".go", L".java", L".sln", L".csproj", L".vcxproj"})
        if (value == kind) return RecentCategory::development;
    return RecentCategory::other;
}

bool is_owned_recent(const CatalogItem& item)
{
    if (item.section != Section::recents) return false;
    return SUCCEEDED(guarded([&] {
        validate_directories();
        const auto path = std::filesystem::path(item.path).lexically_normal();
        if (!path.is_absolute() || !valid_key(path.stem().wstring()) || _wcsicmp(path.extension().c_str(), L".lnk") ||
            _wcsicmp(path.parent_path().c_str(), cache_directory().c_str())) return E_ACCESSDENIED;
        validate_file(path); validate_file(record_file(path));
        return attributes(path) == INVALID_FILE_ATTRIBUTES || attributes(record_file(path)) == INVALID_FILE_ATTRIBUTES ? E_ACCESSDENIED : S_OK;
    }));
}
RecentInfo recent_info(const CatalogItem& item)
{
    if (!is_owned_recent(item)) check(E_ACCESSDENIED);
    const auto file = record_file(item.path);
    return {read(file, L"Name"), filetime(integer(read(file, L"Updated", L"0"))), read(file, L"Pinned", L"0") == L"1"};
}

std::vector<CatalogItem> enumerate_recents()
{
    const auto settings = recent_settings();
    if (!settings.enabled) return {};
    RecentLock lock(true);
    validate_directories();
    if (!lock.acquired()) return cached_items(settings);
    struct Source { std::filesystem::path path; FILETIME time; };
    std::vector<Source> sources;
    const auto directory = windows_recents();
    const auto flags = attributes(directory);
    if (flags != INVALID_FILE_ATTRIBUTES)
    {
        if (!(flags & FILE_ATTRIBUTE_DIRECTORY) || flags & FILE_ATTRIBUTE_REPARSE_POINT) check(E_ACCESSDENIED);
        for (const auto& entry : std::filesystem::directory_iterator(directory))
        {
            const auto path = entry.path();
            if (_wcsicmp(path.extension().c_str(), L".lnk")) continue;
            WIN32_FILE_ATTRIBUTE_DATA data{};
            if (!GetFileAttributesExW(path.c_str(), GetFileExInfoStandard, &data) ||
                data.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_REPARSE_POINT)) continue;
            sources.push_back({path, data.ftLastWriteTime});
        }
    }
    std::sort(sources.begin(), sources.end(), [](const Source& a, const Source& b) { return ticks(a.time) > ticks(b.time); });
    if (sources.size() > recent_limit) sources.resize(recent_limit);
    std::set<std::wstring> seen;
    for (const auto& source : sources)
    {
        // One malformed/stale Windows shortcut must not hide the rest of the list.
        const auto status = guarded([&] {
            auto link = load(source.path);
            const auto target = target_path(link.Get());
            std::array<wchar_t, 32768> arguments{}; check(link->GetArguments(arguments.data(), static_cast<int>(arguments.size())));
            auto identity = lower(target); identity.push_back(L'\0'); identity += arguments.data();
            const auto key = digest(identity);
            if (!seen.insert(key).second) return S_OK; // Newest duplicate wins.
            const auto cache = cache_directory() / (key + L".lnk");
            const auto record = record_file(cache);
            const auto updated = integer(read(record, L"Updated", L"0"));
            const bool missing = attributes(cache) == INVALID_FILE_ATTRIBUTES;
            // Native Shell deletion of a cache shortcut is also interpreted as a local
            // hide, so a subsequent refresh cannot silently resurrect the removed item.
            if (updated && missing && read(record, L"Hidden", L"0") != L"1") write(record, L"Hidden", L"1");
            if (missing || updated != ticks(source.time))
            {
                save_link(link.Get(), cache);
                write(record, L"Name", source.path.stem().wstring());
                write(record, L"Extension", lower(std::filesystem::path(target).extension().wstring()));
                write(record, L"Updated", std::to_wstring(ticks(source.time)));
            }
            if (read(record, L"Present", L"0") != L"1") write(record, L"Present", L"1");
            return S_OK;
        });
        if (FAILED(status) && status != HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) && status != E_INVALIDARG)
            report_error(nullptr, status, IDS_ERROR_RECENTS);
    }
    if (std::filesystem::exists(cache_directory()))
    for (const auto& entry : std::filesystem::directory_iterator(cache_directory()))
    {
        const auto path = entry.path();
        CatalogItem item{Section::recents, path.wstring()};
        if (_wcsicmp(path.extension().c_str(), L".lnk") || !is_owned_recent(item)) continue;
        const auto record = record_file(path);
        if (!seen.contains(path.stem().wstring()) && read(record, L"Present", L"0") != L"0") write(record, L"Present", L"0");
    }
    return cached_items(settings);
}

void hide_recents(const std::vector<CatalogItem>& items)
{
    {
        RecentLock lock; validate_selection(items);
        for (const auto& item : items)
        {
            write(record_file(item.path), L"Hidden", L"1");
            write(record_file(item.path), L"Pinned", L"0");
        }
    }
    notify_launcher();
}
void pin_recents(const std::vector<CatalogItem>& items, bool pin)
{
    {
        RecentLock lock; validate_selection(items);
        for (const auto& item : items) write(record_file(item.path), L"Pinned", pin ? L"1" : L"0");
    }
    notify_launcher();
}
void restore_hidden_recents()
{
    {
        RecentLock lock; validate_directories();
        if (std::filesystem::exists(cache_directory()))
            for (const auto& entry : std::filesystem::directory_iterator(cache_directory()))
            {
                const auto path = entry.path();
                if (valid_key(path.stem().wstring()) && path.extension() == L".ini") write(path, L"Hidden", L"0");
            }
    }
    notify_launcher();
}
void add_recents_to_favorites(const std::vector<CatalogItem>& items)
{
    validate_selection(items);
    for (const auto& item : items)
    {
        // Cache names are opaque hashes; preserve the document's display name instead.
        add_favorite(shell_item(parse_pidl(item.path).get()).Get(), static_cast<Section>(0), recent_info(item).name);
    }
}
}
