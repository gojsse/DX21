cmake_minimum_required(VERSION 3.24)
# JUCE needs C (and Objective-C/C++ on Apple) in addition to C++.
project(OP4 VERSION 1.0.0 LANGUAGES C CXX)

if(APPLE)
  enable_language(OBJC)
  enable_language(OBJCXX)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Workaround for macOS 15 deprecated API warnings in JUCE
if(APPLE)
  add_compile_options(-Wno-error=deprecated-declarations)
endif()

# Fetch JUCE 8
include(FetchContent)
FetchContent_Declare(
  juce
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.14
  GIT_SHALLOW TRUE
)

# Skip building JUCE build tools (juceaide) which has macOS 15 issues
set(JUCE_BUILD_TOOLS OFF CACHE BOOL "")
set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "")

FetchContent_MakeAvailable(juce)

# Fetch ymfm as a submodule (or FetchContent, depending on setup)
# For now, assume it lives in libs/ymfm as a git submodule
set(YMFM_DIR "${CMAKE_CURRENT_SOURCE_DIR}/libs/ymfm")
if(NOT EXISTS "${YMFM_DIR}")
  message(FATAL_ERROR "ymfm submodule not found at ${YMFM_DIR}. Run: git submodule update --init --recursive")
endif()

# OP4 Plugin target
juce_add_plugin(OP4
  COMPANY_NAME "gojsse"
  COMPANY_WEBSITE "https://github.com/gojsse/DX21"
  COMPANY_EMAIL "gojsse@gmail.com"
  PLUGIN_MANUFACTURER_CODE "Gojss"
  PLUGIN_CODE "OP4_"
  FORMATS VST3 AU CLAP Standalone
  IS_SYNTH TRUE
  NEEDS_MIDI_INPUT TRUE
  NEEDS_WEBVIEW2 TRUE   # Windows: link the Edge/Chromium WebView2 for the editor
  COPY_PLUGIN_AFTER_BUILD TRUE
  PRODUCT_NAME "OP4"
)

# Core plugin sources
target_sources(OP4 PRIVATE
  src/PluginProcessor.cpp
  src/PluginProcessor.h
  src/PluginEditor.cpp
  src/PluginEditor.h
)

# Engine & DSP (OPZ core via ymfm's YM2414)
target_sources(OP4 PRIVATE
  src/engine/OPZChip.cpp
  src/engine/OPZChip.h
  src/engine/Voice.h
  src/engine/FMEngine.cpp
  src/engine/FMEngine.h
  src/engine/LFO.h
  src/engine/Envelope.h
  ${YMFM_DIR}/src/ymfm_opz.cpp
)

# Model (Patch, parameter ranges, enums)
target_sources(OP4 PRIVATE
  src/model/Patch.h
  src/model/Types.h
)

# Sysex codecs
target_sources(OP4 PRIVATE
  src/sysex/VCEDCodec.cpp
  src/sysex/VCEDCodec.h
  src/sysex/ACEDCodec.h
  src/sysex/VMEMCodec.h
  src/sysex/SysexRouter.h
)

# MIDI
target_sources(OP4 PRIVATE
  src/midi/NRPNCodec.h
  src/midi/CCTable.h
  src/midi/MEP4Echo.h
)

# Include ymfm
target_include_directories(OP4 PRIVATE ${YMFM_DIR}/src)

# Enable the WebView editor (juce_gui_extra WebBrowserComponent).
# JUCE_USE_WIN_WEBVIEW2 selects the modern Edge/Chromium backend on Windows.
target_compile_definitions(OP4 PRIVATE
  JUCE_WEB_BROWSER=1
  JUCE_USE_WIN_WEBVIEW2=1
  JUCE_VST3_CAN_REPLACE_VST2=0
)

# Link against JUCE + system frameworks
target_link_libraries(OP4 PRIVATE
  juce::juce_core
  juce::juce_audio_basics
  juce::juce_audio_devices
  juce::juce_audio_processors
  juce::juce_audio_utils
  juce::juce_gui_basics
  juce::juce_gui_extra
)

# Platform-specific linking
if(APPLE)
  target_link_libraries(OP4 PRIVATE "-framework CoreAudio" "-framework AudioToolbox" "-framework CoreMIDI")
endif()

# Linux: juce_gui_extra's WebBrowserComponent needs GTK + WebKit, which JUCE does
# NOT auto-provide (no linuxPackages in its module decl). Wire them via pkg-config.
if(UNIX AND NOT APPLE)
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(WEBKIT2GTK REQUIRED IMPORTED_TARGET gtk+-3.0 webkit2gtk-4.1)
  target_link_libraries(OP4 PRIVATE PkgConfig::WEBKIT2GTK)
endif()

# ============================================================================
# Testing — JUCE-free so the DSP/codec cores run fast on every CI platform.
# ============================================================================
enable_testing()

# Engine core: renders the OPZ chip, checks carrier-count + keycode invariants.
add_executable(op4_engine_tests
  tests/engine_tests.cpp
  src/engine/OPZChip.cpp
  ${YMFM_DIR}/src/ymfm_opz.cpp
)
target_compile_features(op4_engine_tests PRIVATE cxx_std_20)
target_include_directories(op4_engine_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${YMFM_DIR}/src)
add_test(NAME op4_engine_tests COMMAND op4_engine_tests)

# Sysex codecs: round-trip fidelity (M2).
add_executable(op4_sysex_tests
  tests/sysex_tests.cpp
  src/sysex/VCEDCodec.cpp
)
target_compile_features(op4_sysex_tests PRIVATE cxx_std_20)
target_include_directories(op4_sysex_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_test(NAME op4_sysex_tests COMMAND op4_sysex_tests)

message(STATUS "OP4 v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Formats: VST3 AU CLAP Standalone")
message(STATUS "ymfm: ${YMFM_DIR}")
