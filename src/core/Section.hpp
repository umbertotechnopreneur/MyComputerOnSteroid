// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; section contract; 2026-10-11.
 * Human guidance: Umberto Giacobbi; native C++, explicit validation evidence.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#pragma once

#include <array>
#include <cstdint>

namespace mcos
{
// Stable identifiers for future persisted favorite metadata, not localized labels.
enum class Section : std::uint32_t
{
    folders = 1,
    drives = 2,
    apps = 3,
    files = 4,
    recents = 5,
};

inline constexpr std::array section_order{
    Section::folders, Section::drives, Section::apps, Section::files, Section::recents};

// A drive is a system item, never a removable favorite.
[[nodiscard]] constexpr bool is_favorite_section(Section section) noexcept
{
    return section == Section::folders || section == Section::apps || section == Section::files;
}
}
