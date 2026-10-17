# Application icon

I use original AI-generated artwork for My Computer Enhanced. The included
artwork is licensed under this repository's MIT license, copyright (c) 2026
Umberto Giacobbi. It is separate from the approved, unchanged VibeWare mark.

The built-in ImageGen tool generated the masters on 2026-10-17 under Umberto
Giacobbi's direction. No human pixel-art or design review is claimed. The small
accepted artwork and the detailed large artwork share the monitor/lightning
identity but are encoded separately for their intended display sizes.

| Asset | Role | SHA-256 |
| --- | --- | --- |
| resources/App-small.png | Small pixel master | 431170C409812FBA41371A1C3E5BEB227FDD382F6B6E5FE569F02645FA1E03A4 |
| resources/App.png | 32-pixel artwork master | A105E85DF76F16F4E765530F8BCF308E64FFF553774EA4D20A2B69BF11BEFA47 |
| resources/App-detailed.png | Detailed high-resolution master | D3362617553AC32E079E7A26EAA8562E0E5E0532CF22F47CDF35823879A0DF85 |
| resources/App.ico | Mixed small/large application icon | 9A7572EB6818A43A01E57F36FC87D7FA0E27BA3DAA20DE3A905751E3C2E9C094 |
| resources/App-512.png | Detailed 512-pixel PNG | 0DE107EAF7651822D157EC85D757E488D4894455B95F1B53305401AA0C5C1D57 |

scripts/Prepare-Icon.py prepares transparent bounds and encodes ten ICO frames:
16/20/24/32/40 use limited palettes and nearest-neighbor pixel artwork;
48/64/96/128/256 use the detailed master with alpha and high-quality resampling.
The accepted 16/24/32 PNG inspection frames were verified byte-for-byte unchanged.
App-48.png and App-256.png are large-frame previews, not application screenshots.

App-512.png is a separate high-definition PNG prepared from the detailed master,
which is already larger than 512 pixels; it is not an enlargement of the small
pixel icon. ICO directory dimensions stop at 256, so no nonstandard 512 ICO entry
is authored. All generated masters remain unchanged. Re-encoding requires Pillow;
native builds consume the checked-in ICO and require no image service or Python.

## Small pixel master prompt

Use case: logo-brand. Asset type: SMALL Windows title-bar icon, authentic Commodore 64 era 8-bit pixel art. This is the tiny 16px version for My Computer Enhanced, a utility that organizes This PC. Draw only a simple desktop monitor with a small chunky lightning bolt on the screen. CRITICAL: design on an EXACT 16 by 16 logical pixel grid, shown enlarged as a square image using nearest-neighbor square blocks. Every logical pixel is one equal sharp square block. All edges follow that grid. The monitor screen and bezel must be easily recognizable at actual 16 by 16 pixels, with a two-pixel-high pedestal. Minimal silhouette, only 4 or 5 solid opaque colors in a classic C64-style palette, at most one-pixel details. Keep lightning extremely simple and legible, 3 chunky stepped segments. Center the icon with only one logical pixel of transparent margin. True transparent background outside the monitor and pedestal. No gradients, no smooth shading, no anti-aliasing, no rounded vector curves, no glows, no texture, no shadows, no glossy effects, no text, no letters, no border around the canvas, no logo. One icon only. This should look like a real 1980s bitmap icon, not a modern icon with a pixel filter.

## 32-pixel master prompt

Use case: stylized-concept. Asset type: larger Windows application icon, 32 logical pixels and above. Input image: style reference for the SAME app icon. Create its crisp 32 by 32 logical-pixel version: a very simple gray desktop monitor with black bezel, a flat blue screen and one thick yellow lightning bolt, and a short pedestal. Authentic Commodore 64 8-bit game/UI pixel art, solid flat colors, hard stair-step pixel edges only. Keep the same simple front-facing monitor and lightning identity as the reference. A dark-gray edge and one light-gray highlight are the only extra detail for this larger version. Absolutely no gradients, metallic reflections, smooth shading, gloss, glows, soft shadow, fine texture, curves, lettering, or modern 3D rendering. Enlarge the 32 by 32 logical pixel grid with nearest-neighbor blocks for the output. Center the icon and fill nearly all the canvas with only one logical pixel of transparent padding. True transparent background outside the icon. One icon, no caption, no contact sheet.

## Detailed large master prompt

Use case: logo-brand. Asset type: highly detailed Windows desktop application icon for LARGE sizes 48, 64, 96, 128 and 256 pixels, plus a high-definition 512px PNG. Input image is the original app icon reference; preserve its identity: a front-facing desktop computer monitor with a short stand, deep blue screen and one bold bright yellow lightning bolt centered on the display. Create a refined, beautifully detailed large-size version, smooth crisp contours, carefully constructed dark graphite and brushed aluminum monitor casing, clean bevels, subtle controlled reflections and depth. Blue screen has restrained layered flowing light shapes; lightning is rich golden yellow with a clearly defined edge and subtle dimensionality. Real Windows desktop app icon aesthetics, not photorealistic product photography. Sophisticated but readable broad silhouette, no extra tiny gadgets. The larger icon can have much more detail than a tiny pixel-art icon. Keep same front-facing monitor-and-lightning concept. Composition: centered, nearly square icon, tightly framed with about 5 percent transparent padding and the stand fully inside the canvas. Transparent background outside the monitor and stand. No backdrop, no plate behind it, no text, no letters, no watermark, no pixelation. Produce a high-resolution master with clean detail that remains sharp at 512px; one icon only.
