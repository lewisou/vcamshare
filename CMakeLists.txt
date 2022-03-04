cmake_minimum_required(VERSION 3.10)

# set the project name
project(cppshare VERSION 0.1)

configure_file(./src/config.h.in config.h)

set(main_sources
    muxer.cpp
)
list(TRANSFORM main_sources PREPEND "src/main/")

set(test_sources
    example_test.cpp
)
list(TRANSFORM test_sources PREPEND "src/test/")

add_library(${CMAKE_PROJECT_NAME} SHARED ${main_sources})
set_property(TARGET ${CMAKE_PROJECT_NAME} PROPERTY CXX_STANDARD 11)

add_executable(test ${test_sources})
set_property(TARGET test PROPERTY CXX_STANDARD 11)

target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )

target_include_directories(test PUBLIC
"${PROJECT_BINARY_DIR}"
"${PROJECT_SOURCE_DIR}/libs/boost/include/"
)

target_link_libraries(test ${CMAKE_PROJECT_NAME})

message(PLATFORM="${PLATFORM}")