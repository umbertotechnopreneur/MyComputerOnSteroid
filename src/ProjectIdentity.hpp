// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; project identity; 2026-10-11.
 * Human guidance: Umberto Giacobbi; native C++, explicit validation evidence.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#pragma once

#include <guiddef.h>

namespace mcos::identity
{
inline constexpr wchar_t product_name[] = L"MyComputerOnSteroid";
inline constexpr wchar_t creator[] = L"Umberto Giacobbi";
inline constexpr wchar_t publisher[] = L"umbertogiacobbi.biz";
inline constexpr wchar_t website[] = L"https://umbertogiacobbi.biz";
inline constexpr wchar_t manifesto[] = L"https://umbertogiacobbi.biz/vibeware/manifesto";

// Stable identity of the launcher root. Favorites do not each receive a CLSID.
inline constexpr CLSID launcher_clsid{
    0xb36b9212, 0x2d35, 0x4e65, {0x8a, 0x97, 0x0a, 0xa5, 0xd2, 0x80, 0xe1, 0x57}};
}
