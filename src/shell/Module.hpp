// SPDX-License-Identifier: MIT
/* VBWR B
 * Project: MyComputerOnSteroid
 * Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
 * Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
 * VibeWare participation: Human intent, AI execution, and plenty of tokens
 * Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
 * Created with AI: OpenAI Codex; COM module lifetime accounting; 2026-10-11.
 * Human guidance: Umberto Giacobbi; native C++, explicit validation evidence.
 * Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
 * Copyright (c) 2026 Umberto Giacobbi
 * License: MIT
 * VBWR E */

#pragma once

#include <Windows.h>
#include <atomic>

namespace mcos::shell
{
inline std::atomic<long> live_objects{0};
inline std::atomic<long> server_locks{0};

// iid: requested interface.
// result: receives one owned interface reference; null on failure.
HRESULT create_shell_folder(REFIID iid, void** result) noexcept;
}
