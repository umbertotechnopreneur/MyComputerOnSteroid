# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Umberto Giacobbi
cmake_minimum_required(VERSION 3.25)
project(MyComputerOnSteroid LANGUAGES CXX RC)

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
target_link_libraries(MyComputerOnSteroid.Shell PRIVATE shell32 shlwapi ole32 oleaut32 uuid)
set_target_properties(MyComputerOnSteroid.Shell PROPERTIES
    CXX_EXTENSIONS OFF
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
