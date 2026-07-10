cmake_minimum_required(VERSION 3.24)
project(OP4 VERSION 1.0.0 LANGUAGES CXX)

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
  GIT_TAG 7.0.12
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
  COPY_PLUGIN_AFTER_BUILD TRUE
  PRODUCT_NAME "OP4"
)

# Core plugin sources
target_sources(OP4 PRIVATE
  src/PluginProcessor.cpp
  src/PluginProcessor.h
  src/PluginEditor.h
)

# Engine & DSP
target_sources(OP4 PRIVATE
  src/engine/Voice.cpp
  src/engine/Voice.h
  src/engine/FMEngine.cpp
  src/engine/FMEngine.h
  src/engine/LFO.h
  src/engine/Envelope.h
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

# Link against JUCE + system frameworks
target_link_libraries(OP4 PRIVATE juce::juce_core juce::juce_audio_basics juce::juce_audio_devices juce::juce_gui_basics)

# Platform-specific linking
if(APPLE)
  target_link_libraries(OP4 PRIVATE "-framework CoreAudio" "-framework AudioToolbox" "-framework CoreMIDI")
endif()

# ============================================================================
# Testing (optional, for later)
# ============================================================================
enable_testing()
add_executable(op4_tests
  tests/sysex_tests.cpp
  tests/engine_tests.cpp
)
target_include_directories(op4_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${YMFM_DIR}/src)
target_link_libraries(op4_tests PRIVATE juce::juce_core juce::juce_audio_basics)
add_test(NAME op4_tests COMMAND op4_tests)

message(STATUS "OP4 v${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Formats: VST3 AU CLAP Standalone")
message(STATUS "ymfm: ${YMFM_DIR}")
