cmake_minimum_required(VERSION 3.15)
project(AudioPlayer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find JUCE
add_subdirectory(3rdparty/JUCE JUCE)

# Include directories
include_directories(include)
include_directories(src)

# Source files
set(SOURCES
    src/player.cpp
    src/effect.cpp
)

# Header files
set(HEADERS
    include/player.h
    include/effect.h
)

# Create library
add_library(
    ${PROJECT_NAME}
    STATIC
    ${SOURCES}
    ${HEADERS})

# 启用MP3格式支持
target_compile_definitions(${PROJECT_NAME}
    PUBLIC
        JUCE_USE_CURL=0
        JUCE_WEB_BROWSER=0
        JUCE_USE_FLAC=1
        JUCE_USE_OGGVORBIS=1
        JUCE_USE_MP3AUDIOFORMAT=1
        JUCE_USE_LAME_AUDIO_FORMAT=1
        JUCE_USE_WINDOWS_MEDIA_FORMAT=1
)

# Link JUCE modules
target_link_libraries(${PROJECT_NAME}
    PUBLIC
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_core
        juce::juce_audio_basics
)

# Platform-specific linking
if(WIN32)
    target_link_libraries(${PROJECT_NAME} 
        PUBLIC
            winmm
            ws2_32
    )
elseif(APPLE)
    target_link_libraries(${PROJECT_NAME} 
        PUBLIC
            "-framework CoreAudio"
            "-framework CoreMIDI"
            "-framework AudioToolbox"
    )
elseif(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME}
        PUBLIC
            asound
            pthread
            curl
    )
endif()

# Create executable
add_executable(
    ${PROJECT_NAME}_main 
    src/main.cpp 
    ${HEADERS})

target_link_libraries(
    ${PROJECT_NAME}_main
    ${PROJECT_NAME})

