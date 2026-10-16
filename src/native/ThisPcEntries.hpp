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

namespace mcos
{
// Opens the elevated native dialog that owns direct This PC folder registrations.
// The caller can invoke it from the launcher without needing Registry write access.
// Only current MCOS-owned registrations are read.
HRESULT launch_this_pc_entry_manager();

// Adds one or more real folders directly to This PC. If the launcher is not
// elevated, it asks once and then continues with every supplied folder.
HRESULT add_this_pc_folders(const std::vector<std::wstring>& paths);
}
