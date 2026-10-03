cmake_minimum_required(VERSION 3.22)

# static MSVC runtime -> no VC++ redistributable needed on the target PC
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

project(SishhinHZMachine VERSION 1.0.0 LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.6
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(JUCE)

# JUCE's JS bridge (window.__JUCE__ helpers) is embedded next to our UI, served at /js/juce/index.js
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/uigen)
file(COPY ${juce_SOURCE_DIR}/modules/juce_gui_extra/native/javascript/index.js DESTINATION ${CMAKE_BINARY_DIR}/uigen)

juce_add_binary_data(UIData
    HEADER_NAME UIData.h
    NAMESPACE   UIData
    SOURCES     Source/ui/index.html
                ${CMAKE_BINARY_DIR}/uigen/index.js)

juce_add_plugin(SishhinHZMachine
    COMPANY_NAME               "SISHHIN"
    PLUGIN_MANUFACTURER_CODE   Sshn
    PLUGIN_CODE                Hzm1
    FORMATS                    VST3
    PRODUCT_NAME               "SISHHIN HZ MACHINE"
    IS_SYNTH                   FALSE
    NEEDS_MIDI_INPUT           FALSE
    NEEDS_MIDI_OUTPUT          FALSE
    VST3_CATEGORIES            Fx Distortion
    COPY_PLUGIN_AFTER_BUILD    FALSE
    NEEDS_WEB_BROWSER          TRUE
    NEEDS_WEBVIEW2             TRUE)

target_sources(SishhinHZMachine PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp)

target_compile_definitions(SishhinHZMachine PUBLIC
    JUCE_WEB_BROWSER=1
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_USE_WIN_WEBVIEW2_WITH_STATIC_LINKING=1)

target_include_directories(SishhinHZMachine PRIVATE Source)

target_link_libraries(SishhinHZMachine
    PRIVATE
        UIData
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
