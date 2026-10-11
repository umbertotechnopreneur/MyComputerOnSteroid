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

#include "MenuIcons.hpp"
#include <algorithm>
#include <cstdint>

namespace mcos
{
namespace
{
struct DeleteDC
{
    using pointer = HDC;
    void operator()(HDC dc) const noexcept { ::DeleteDC(dc); }
};
struct DeleteFont
{
    using pointer = HFONT;
    void operator()(HFONT font) const noexcept { DeleteObject(font); }
};
struct SelectObjectBack
{
    HDC dc;
    HGDIOBJ previous;
    ~SelectObjectBack() { if (previous && previous != HGDI_ERROR) SelectObject(dc, previous); }
};

COLORREF menu_color() noexcept
{
    HIGHCONTRASTW contrast{sizeof(contrast)};
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(contrast), &contrast, 0) &&
        (contrast.dwFlags & HCF_HIGHCONTRASTON)) return GetSysColor(COLOR_MENUTEXT);

    DWORD light = 1, size = sizeof(light);
    RegGetValueW(HKEY_CURRENT_USER,
        L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
        L"AppsUseLightTheme", RRF_RT_REG_DWORD, nullptr, &light, &size);
    return light ? RGB(60, 60, 60) : RGB(225, 225, 225);
}

MenuBitmap render(MenuGlyph glyph, int pixels, COLORREF color) noexcept
{
    std::unique_ptr<void, DeleteDC> dc(CreateCompatibleDC(nullptr));
    if (!dc) return {};
    BITMAPINFO info{};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = pixels;
    info.bmiHeader.biHeight = -pixels;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    void* raw = nullptr;
    MenuBitmap bitmap(CreateDIBSection(dc.get(), &info, DIB_RGB_COLORS, &raw, nullptr, 0));
    if (!bitmap || !raw) return {};
    SelectObjectBack restore_bitmap{dc.get(), SelectObject(dc.get(), bitmap.get())};
    if (!restore_bitmap.previous || restore_bitmap.previous == HGDI_ERROR) return {};

    for (PCWSTR face : {L"Segoe Fluent Icons", L"Segoe MDL2 Assets"})
    {
        // Windows Server CI and older system fonts do not have Fluent Hide.
        // Preserve a visible native symbol without weakening the menu tests.
        const auto compatible = glyph == MenuGlyph::hide && _wcsicmp(face, L"Segoe MDL2 Assets") == 0
            ? MenuGlyph::blocked : glyph;
        const wchar_t character = static_cast<wchar_t>(compatible);
        std::unique_ptr<void, DeleteFont> font(CreateFontW(-pixels, 0, 0, 0, FW_NORMAL,
            FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            ANTIALIASED_QUALITY, DEFAULT_PITCH, face));
        if (!font) continue;
        SelectObjectBack restore_font{dc.get(), SelectObject(dc.get(), font.get())};
        if (!restore_font.previous || restore_font.previous == HGDI_ERROR) continue;
        wchar_t actual_face[LF_FACESIZE]{};
        WORD glyph_index = 0xffff;
        if (!GetTextFaceW(dc.get(), ARRAYSIZE(actual_face), actual_face) || _wcsicmp(actual_face, face) != 0 ||
            GetGlyphIndicesW(dc.get(), &character, 1, &glyph_index, GGI_MARK_NONEXISTING_GLYPHS) == GDI_ERROR ||
            glyph_index == 0xffff) continue;

        auto* bits = static_cast<std::uint32_t*>(raw);
        std::fill_n(bits, pixels * pixels, 0x00ffffffu);
        SetBkMode(dc.get(), TRANSPARENT);
        SetTextColor(dc.get(), RGB(0, 0, 0));
        RECT rectangle{0, 0, pixels, pixels};
        if (!DrawTextW(dc.get(), &character, 1, &rectangle, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX)) continue;
        GdiFlush();
        // GDI text supplies a grayscale coverage mask. Native menus consume a
        // top-down 32-bit DIB with premultiplied alpha, including hover states.
        bool visible = false;
        for (int i = 0; i < pixels * pixels; ++i)
        {
            const auto gray = ((bits[i] & 0xff) + ((bits[i] >> 8) & 0xff) + ((bits[i] >> 16) & 0xff)) / 3;
            const auto alpha = 255 - gray;
            visible = visible || alpha != 0;
            bits[i] = (alpha << 24) | ((GetRValue(color) * alpha / 255) << 16) |
                ((GetGValue(color) * alpha / 255) << 8) | (GetBValue(color) * alpha / 255);
        }
        if (visible) return bitmap;
    }
    return {};
}
}

bool MenuIcons::attach(HMENU menu, UINT command, MenuGlyph glyph, HWND owner) noexcept
{
    // Decoration is optional. Missing fonts or GDI/allocation failure must not
    // suppress a native menu, its accessible text or its working commands.
    try
    {
        const UINT dpi = owner && IsWindow(owner) ? GetDpiForWindow(owner) : 96;
        const int pixels = std::clamp(GetSystemMetricsForDpi(SM_CXSMICON, dpi ? dpi : 96), 16, 128);
        const auto color = menu_color();
        auto entry = std::find_if(entries_.begin(), entries_.end(), [&](const auto& value) {
            return value.glyph == glyph && value.pixels == pixels && value.color == color;
        });
        if (entry == entries_.end())
        {
            auto bitmap = render(glyph, pixels, color);
            if (!bitmap) return false;
            entries_.push_back({glyph, pixels, color, std::move(bitmap)});
            entry = std::prev(entries_.end());
        }
        MENUITEMINFOW item{sizeof(item)};
        item.fMask = MIIM_BITMAP;
        item.hbmpItem = entry->bitmap.get();
        return SetMenuItemInfoW(menu, command, FALSE, &item) != FALSE;
    }
    catch (...) { return false; }
}
}
