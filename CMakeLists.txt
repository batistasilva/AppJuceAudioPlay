cmake_minimum_required(VERSION 3.22)
project(AppJuceAudioPlay VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(JUCE_DIR "/home/WorkSpace/Aplication-JUCE/JUCE")
add_subdirectory("${JUCE_DIR}" "${CMAKE_BINARY_DIR}/JUCE")

juce_add_gui_app(AppJuceAudioPlay
    VERSION 1.0.0
    PRODUCT_NAME "AppJuceAudioPlay"
    NEEDS_WEB_BROWSER TRUE
)

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    set_target_properties(AppJuceAudioPlay PROPERTIES
        JUCE_ANDROID_PACKAGE_NAME "com.exemplo.AppJuceAudioPlay"
        JUCE_ANDROID_MIN_SDK_VERSION 29
        JUCE_ANDROID_TARGET_SDK_VERSION 34
    )
endif()

juce_generate_juce_header(AppJuceAudioPlay)

target_sources(AppJuceAudioPlay
    PRIVATE
        "Source/MainComponent.cpp"
        "Source/MainComponent.h"
        "Source/Main.cpp"
)

target_compile_definitions(AppJuceAudioPlay
    PRIVATE
        JUCE_WEB_BROWSER=1
        JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:AppJuceAudioPlay,JUCE_PRODUCT_NAME>"
        JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:AppJuceAudioPlay,JUCE_VERSION>"
        JUCE_USE_MP3AUDIOFORMAT=1
)

target_link_libraries(AppJuceAudioPlay
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_basics
        juce::juce_audio_formats
        juce::juce_audio_devices
        juce::juce_dsp
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WEBKIT REQUIRED webkit2gtk-4.1)
    target_include_directories(AppJuceAudioPlay PRIVATE ${WEBKIT_INCLUDE_DIRS})
    target_link_libraries(AppJuceAudioPlay PRIVATE ${WEBKIT_LIBRARIES})
endif()
