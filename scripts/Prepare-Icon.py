# SPDX-License-Identifier: MIT
# VBWR B
# Project: MyComputerOnSteroid
# Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
# Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
# VibeWare participation: Human intent, AI execution, and plenty of tokens
# Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
# Created with AI: OpenAI Codex; reproducible Windows ICO encoding; 2026-10-17.
# Human guidance: Umberto Giacobbi; one icon for app, dialogs and installer.
# Evidence: docs/VALIDATION.md; no human code review claimed.
# Copyright (c) 2026 Umberto Giacobbi
# License: MIT
# VBWR E

"""Encode small pixel artwork and detailed large artwork in one Windows ICO."""

from pathlib import Path
from io import BytesIO
import struct
from PIL import Image, ImageOps

resource_directory = Path(__file__).resolve().parent.parent / "resources"
sizes = [(edge, edge) for edge in (16, 20, 24, 32, 40, 48, 64, 96, 128, 256)]
frames = []
for filename, grid, colors in (("App-small.png", 16, 5), ("App.png", 32, 8)):
    with Image.open(resource_directory / filename) as master:
        if master.mode != "RGBA" or master.getextrema()[3][0] != 0:
            raise ValueError("The generated icon masters must preserve transparency")
        # ICO preparation: fit the transparent artwork to its intended pixel grid
        # and encode a limited, undithered palette. No smooth resizing is used.
        visible_bounds = master.getchannel("A").point(lambda alpha: 255 if alpha >= 128 else 0).getbbox()
        content = master.crop(visible_bounds)
        fitted = ImageOps.contain(content, (grid - 2, grid - 2), Image.Resampling.NEAREST)
        canvas = Image.new("RGBA", (grid, grid))
        canvas.paste(fitted, ((grid - fitted.width) // 2, (grid - fitted.height) // 2))
        bitmap = canvas.quantize(colors=colors + 1, method=Image.Quantize.FASTOCTREE, dither=Image.Dither.NONE)
        for edge, _ in sizes:
            if edge >= 48 or (edge < 32) != (grid == 16):
                continue
            frame = bitmap.resize((edge, edge), Image.Resampling.NEAREST)
            if edge in (16, 24, 32):
                frame.save(resource_directory / f"App-{edge}.png")
            stream = BytesIO()
            frame.save(stream, format="PNG")
            frames.append((edge, stream.getvalue()))

with Image.open(resource_directory / "App-detailed.png") as master:
    if master.mode != "RGBA" or master.getextrema()[3][0] != 0:
        raise ValueError("The detailed icon master must preserve transparency")
    visible_bounds = master.getchannel("A").point(lambda alpha: 255 if alpha >= 128 else 0).getbbox()
    content = master.crop(visible_bounds)
    for edge in (48, 64, 96, 128, 256, 512):
        margin = max(2, edge // 32)
        fitted = ImageOps.contain(content, (edge - 2 * margin, edge - 2 * margin), Image.Resampling.LANCZOS)
        frame = Image.new("RGBA", (edge, edge))
        frame.paste(fitted, ((edge - fitted.width) // 2, (edge - fitted.height) // 2))
        if edge in (48, 256, 512):
            frame.save(resource_directory / f"App-{edge}.png")
        if edge <= 256:
            stream = BytesIO()
            frame.save(stream, format="PNG")
            frames.append((edge, stream.getvalue()))

frames.sort(key=lambda item: item[0])

# Each size has its own pre-rendered PNG frame. A general-purpose ICO writer's
# default smooth resampling would blur the deliberately separate small artwork.
offset = 6 + 16 * len(frames)
directory = bytearray(struct.pack("<HHH", 0, 1, len(frames)))
for edge, payload in frames:
    directory.extend(struct.pack("<BBBBHHII", edge % 256, edge % 256, 0, 0, 1, 32, len(payload), offset))
    offset += len(payload)
with (resource_directory / "App.ico").open("wb") as output:
    output.write(directory)
    for _, payload in frames:
        output.write(payload)
with Image.open(resource_directory / "App.ico") as encoded:
    if encoded.ico.sizes() != set(sizes):
        raise ValueError("The ICO does not contain every requested size")
print("Encoded ten ICO sizes: pixel art below 48px, detailed artwork at 48-256px; separate 512px PNG")
