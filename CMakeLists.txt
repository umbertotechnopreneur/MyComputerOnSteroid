# SPDX-License-Identifier: MIT
# VBWR B
# Project: MyComputerOnSteroid
# Repository: https://github.com/umbertotechnopreneur/MyComputerOnSteroid
# Creator: Umberto Giacobbi | https://umbertogiacobbi.biz
# VibeWare participation: Human intent, AI execution, and plenty of tokens
# Manifesto: https://umbertogiacobbi.biz/vibeware/manifesto
# Created with AI: OpenAI Codex; native targets and compiler contracts; 2026-10-11.
# Human guidance: Umberto Giacobbi; native C++, Win32 and COM.
# Evidence: docs/VALIDATION.md and Git history; no human code review claimed.
# Copyright (c) 2026 Umberto Giacobbi
# License: MIT
# VBWR E
cmake_minimum_required(VERSION 3.25)
project(MyComputerOnSteroid LANGUAGES CXX RC)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

if(NOT WIN32 OR NOT MSVC)
    message(FATAL_ERROR "Use the Visual Studio C++ toolchain and Windows SDK.")
endif()
if(NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "Only native Windows x64 and ARM64 builds are supported.")
endif()

add_library(MyComputerOnSteroid.Shell SHARED
    src/ProjectIdentity.hpp
    src/core/Section.hpp
    src/shell/Module.hpp
    src/shell/Dll.cpp
    src/shell/ShellFolder.hpp
    src/shell/ShellFolder.cpp
    src/core/Catalog.cpp
    src/core/Recents.cpp
    src/core/ViewSettings.cpp
    src/native/Support.cpp
    src/native/RecentDialog.cpp
    src/native/ViewDialog.cpp
    src/native/MenuIcons.cpp
    src/shell/ItemId.cpp
    src/shell/Grouping.cpp
    src/shell/FavoriteMenu.cpp
    src/shell/MyComputerOnSteroid.def
    resources/resource.h
    resources/Shell.rc
)
target_compile_features(MyComputerOnSteroid.Shell PRIVATE cxx_std_20)
target_compile_definitions(MyComputerOnSteroid.Shell PRIVATE
    UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX
    WINVER=0x0A00 _WIN32_WINNT=0x0A00
)
target_compile_options(MyComputerOnSteroid.Shell PRIVATE
    "$<$<COMPILE_LANGUAGE:CXX>:/W4;/WX;/permissive-;/utf-8;/EHsc;/guard:cf;/sdl>"
)
target_link_options(MyComputerOnSteroid.Shell PRIVATE /DYNAMICBASE /NXCOMPAT /GUARD:CF)
target_link_libraries(MyComputerOnSteroid.Shell PRIVATE shell32 shlwapi ole32 oleaut32 uuid propsys bcrypt)
set_target_properties(MyComputerOnSteroid.Shell PROPERTIES
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

add_executable(MyComputerOnSteroid WIN32 src/Launcher.cpp src/core/Catalog.cpp src/core/Recents.cpp src/native/Support.cpp resources/Shell.rc)
add_executable(MyComputerOnSteroid.Tests tests/NativeTests.cpp src/core/Catalog.cpp src/core/Recents.cpp src/core/ViewSettings.cpp src/native/Support.cpp src/shell/ItemId.cpp resources/Shell.rc)
foreach(target MyComputerOnSteroid MyComputerOnSteroid.Tests)
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_compile_definitions(${target} PRIVATE MCOS_EXECUTABLE UNICODE _UNICODE WIN32_LEAN_AND_MEAN NOMINMAX WINVER=0x0A00 _WIN32_WINNT=0x0A00)
    target_compile_options(${target} PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:/W4;/WX;/permissive-;/utf-8;/EHsc;/guard:cf;/sdl>")
    target_link_options(${target} PRIVATE /DYNAMICBASE /NXCOMPAT /GUARD:CF)
    target_link_libraries(${target} PRIVATE shell32 shlwapi ole32 oleaut32 uuid propsys bcrypt)
    set_target_properties(${target} PROPERTIES CXX_EXTENSIONS OFF RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endforeach()
