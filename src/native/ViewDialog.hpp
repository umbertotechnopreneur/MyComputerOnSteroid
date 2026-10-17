// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; native search/options dialog; 2026-10-11.
 * Human guidance: Umberto Giacobbi; ordinary Win32 controls.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */
#pragma once
#include "../core/ViewSettings.hpp"
#include "Dialog.hpp"
namespace mcos
{
std::unique_ptr<NativeDialogPage> create_view_page(HWND parent, ViewSettings& settings);
}
