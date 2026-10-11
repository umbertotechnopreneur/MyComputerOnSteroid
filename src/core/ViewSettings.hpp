// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; collection search and family policy; 2026-10-11.
 * Human guidance: Umberto Giacobbi; native groups and reference-only search.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */
#pragma once
#include "Catalog.hpp"

namespace mcos
{
enum class GroupMode : unsigned { sections, extension, families, none };
enum class SearchField : unsigned { any, name, path, extension };
enum class ItemOrder : unsigned { name, date };
struct ExtensionFamily { std::wstring name; std::vector<std::wstring> extensions; };
struct ViewSettings
{
    GroupMode grouping{GroupMode::sections};
    ItemOrder order{ItemOrder::date};
    bool descending{true};
    SearchField field{SearchField::any};
    std::wstring query;
    std::wstring families;
};
ViewSettings view_settings();
void save_view_settings(const ViewSettings& settings);
std::vector<ExtensionFamily> parse_families(const std::wstring& definitions);
std::wstring item_target_path(const CatalogItem& item);
std::wstring item_extension(const CatalogItem& item);
std::wstring item_family(const CatalogItem& item, const std::vector<ExtensionFamily>& families);
FILETIME item_date(const CatalogItem& item);
bool matches_search(const CatalogItem& item, const ViewSettings& settings);
}
