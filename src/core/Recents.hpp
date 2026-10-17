// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; optional recent-item policy; 2026-10-11.
 * Human guidance: Umberto Giacobbi; filtering, dates and bulk pin/hide in Explorer.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#pragma once
#include "Catalog.hpp"

namespace mcos
{
enum class RecentCategory : unsigned { all, documents, images, audio, video, development, other };
struct RecentSettings
{
    bool enabled{false};
    std::wstring name;
    RecentCategory category{RecentCategory::all};
    std::wstring extension;
};
struct RecentInfo
{
    std::wstring name;
    FILETIME updated{}; // LastWriteTime of the Windows Recent shortcut, NOT target access time.
    bool pinned{false};
};

RecentSettings recent_settings();
// Validation has no I/O, so a settings host can validate every page before saving.
void validate_recent_settings(const RecentSettings& settings);
void save_recent_settings(const RecentSettings& settings);
std::vector<CatalogItem> enumerate_recents();
bool is_owned_recent(const CatalogItem& item);
RecentInfo recent_info(const CatalogItem& item);
RecentCategory recent_category(const std::wstring& extension);
void hide_recents(const std::vector<CatalogItem>& items);
void pin_recents(const std::vector<CatalogItem>& items, bool pin);
void restore_hidden_recents();
void add_recents_to_favorites(const std::vector<CatalogItem>& items);
}
