// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; native This PC entry manager; 2026-10-16.
 * Human guidance: Umberto Giacobbi; native management of folders shown directly in This PC.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#pragma once

#include <Windows.h>
#include <string>
#include <vector>
#include "Dialog.hpp"

namespace mcos
{
// Adds one or more real folders directly to This PC. If the launcher is not
// elevated, it asks once and then continues with every supplied folder.
HRESULT add_this_pc_folders(const std::vector<std::wstring>& paths);

std::unique_ptr<NativeDialogPage> create_this_pc_page(HWND parent);

// Elevated worker entry point: validates the caller SID and current-format
// ownership before touching exact registration keys; never opens another UI.
HRESULT run_this_pc_worker(const std::vector<std::wstring>& arguments);
}
