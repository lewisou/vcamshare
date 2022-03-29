cmake_minimum_required(VERSION 3.10)

# set the project name
project(cppshare VERSION 0.1)

configure_file(./src/config.h.in config.h)

set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
# set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
# set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)

# src folder
set(main_sources
    video_muxer.cpp
)
list(TRANSFORM main_sources PREPEND "src/main/")

set(test_sources
    example_test.cpp
)
list(TRANSFORM test_sources PREPEND "src/test/")
message(APPLE="${APPLE}")

# Find library
if(APPLE)
    find_library(AVFORMAT avformat ${PROJECT_SOURCE_DIR}/libs/ffmpeg/macos)
    find_library(AVUTIL avutil ${PROJECT_SOURCE_DIR}/libs/ffmpeg/macos)
    find_library(AVCODEC avcodec ${PROJECT_SOURCE_DIR}/libs/ffmpeg/macos)
elseif(ANDROID)
    find_library(AVFORMAT avformat ${PROJECT_SOURCE_DIR}/libs/ffmpeg/android/${ANDROID_ABI} NO_DEFAULT_PATH)
    find_library(AVUTIL avutil ${PROJECT_SOURCE_DIR}/libs/ffmpeg/android/${ANDROID_ABI} NO_DEFAULT_PATH)
    find_library(AVCODEC avcodec ${PROJECT_SOURCE_DIR}/libs/ffmpeg/android/${ANDROID_ABI} NO_DEFAULT_PATH)

    # find_library(AVFORMAT 
    #             NAMES avformat
    #             PATHS ${PROJECT_SOURCE_DIR}/libs/ffmpeg/android/${ANDROID_ABI}
    #             NO_DEFAULT_PATH)
endif()

# The target itself
add_library(${CMAKE_PROJECT_NAME} SHARED ${main_sources})
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 11)

# include folders
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/libs/boost/include/"
    "${PROJECT_SOURCE_DIR}/libs/FFmpeg/include/"
)

# link for the main lib
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE 
                        ${AVFORMAT}
                        ${AVUTIL}
                        ${AVCODEC}
                        )


# Test executable ======================================

add_executable(test ${test_sources})
set_property(TARGET test PROPERTY CXX_STANDARD 11)

target_include_directories(test PUBLIC
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/libs/boost/include/"
    "${PROJECT_SOURCE_DIR}/libs/FFmpeg/include/"
)

target_link_libraries(test PRIVATE ${CMAKE_PROJECT_NAME})
