@REM SPDX-License-Identifier: MIT
@REM VBWR B
@REM
@REM Project: MyComputerOnSteroid
@REM Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
@REM Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
@REM
@REM VibeWare participation: Human intent, AI execution, and plenty of tokens
@REM Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
@REM
@REM Modified with AI: OpenAI Codex; corrected project identity and repository-root launch on 2026-10-19.
@REM Human guidance: Umberto Giacobbi; requested area-based commits and publication.
@REM
@REM Copyright (c) 2026 Umberto Giacobbi
@REM License: MIT
@REM
@REM VBWR E

@REM VS Code workspace launcher

@echo off
setlocal

pushd "%~dp0.." >nul || exit /b 1

where code >nul 2>&1
if errorlevel 1 (
	echo VS Code command-line launcher was not found on PATH.
	echo Install the "code" command in PATH, then run this file again.
	popd >nul
	exit /b 1
)

start "" code .

popd >nul
exit /b 0
