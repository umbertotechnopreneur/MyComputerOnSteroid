// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; native favorite menu adapter; 2026-10-11.
 * Human guidance: Umberto Giacobbi; native menus, remove references only.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#pragma once
#include "../core/Catalog.hpp"
#include <vector>

namespace mcos::shell
{
// Borrow native; the returned menu owns its own reference and validated item snapshot.
HRESULT favorite_menu(IContextMenu* native, std::vector<CatalogItem> items, HWND owner,
    REFIID iid, void** result) noexcept;
}
