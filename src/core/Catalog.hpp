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

#pragma once
#include "Section.hpp"
#include "../native/Support.hpp"
#include <vector>

namespace mcos
{
struct CatalogItem
{
    Section section{};
    std::wstring path; // Drive root or an owned .lnk; never substitute a favorite's target.
};

std::filesystem::path data_root();
std::filesystem::path section_directory(Section section);
std::vector<CatalogItem> enumerate_catalog(bool include_recents = true);
Section classify(IShellItem* target);
std::filesystem::path add_favorite(IShellItem* target, Section requested = static_cast<Section>(0), const std::wstring& name = {});
bool is_owned_favorite(const CatalogItem& item);
void remove_favorite(const CatalogItem& item);
CatalogItem rename_favorite(const CatalogItem& item, const std::wstring& name);
void change_favorite_icon(const CatalogItem& item, const std::filesystem::path& icon);
void choose_favorite_icon(HWND owner, const std::vector<CatalogItem>& items);
Pidl navigation_target(const CatalogItem& item);
std::wstring item_name(const CatalogItem& item);
void choose_favorite(HWND owner, Section section);
}
