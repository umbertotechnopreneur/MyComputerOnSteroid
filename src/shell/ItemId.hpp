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
#include "../core/Catalog.hpp"

namespace mcos::shell
{
Pidl make_item_id(const CatalogItem& item);
CatalogItem read_item_id(PCUIDLIST_RELATIVE pidl);
HRESULT native_ui_object(HWND owner, UINT count, PCUITEMID_CHILD_ARRAY items, REFIID iid, void** result);
// Shell identities retain owned references; file-transfer formats expose document
// targets for receivers that do not understand Shell links (for example uploads).
HRESULT transfer_data_object(UINT count, PCUITEMID_CHILD_ARRAY items, IDataObject** result) noexcept;
}
