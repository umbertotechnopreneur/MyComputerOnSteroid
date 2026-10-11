// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; native menu glyphs; 2026-10-11.
 * Human guidance: Umberto Giacobbi; Windows 11 style on native menu commands.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#pragma once
#include "Support.hpp"
#include <vector>

namespace mcos
{
// Public system-font code points. Older versions may lack the Hide glyph;
// the Remove symbol is tried only when Hide cannot render.
enum class MenuGlyph : wchar_t
{
    favorite = 0xe734, pin = 0xe718, unpin = 0xe77a, hide = 0xed1a, remove = 0xe738,
    picture = 0xeb9f, folder = 0xe8b7, folder_open = 0xe838,
    apps = 0xe71d, document = 0xe8a5, filter = 0xe71c,
    history = 0xe81c, restore = 0xe7a7
};

struct DeleteMenuBitmap
{
    using pointer = HBITMAP;
    void operator()(HBITMAP bitmap) const noexcept { DeleteObject(bitmap); }
};
using MenuBitmap = std::unique_ptr<void, DeleteMenuBitmap>;

// The menu borrows these bitmaps. Keep this owner alive until its HMENUs close.
// Cache old DPI/theme variants too: a nested menu may still refer to them.
class MenuIcons
{
public:
    bool attach(HMENU menu, UINT command, MenuGlyph glyph, HWND owner) noexcept;
private:
    struct Entry { MenuGlyph glyph; int pixels; COLORREF color; MenuBitmap bitmap; };
    std::vector<Entry> entries_;
};
}
