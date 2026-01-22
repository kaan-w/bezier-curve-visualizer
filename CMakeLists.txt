cmake_minimum_required(VERSION 3.16)
project(bezier-curve-visualizer C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

include(CheckIncludeFile)
include(FetchContent)

add_executable(${PROJECT_NAME} src/main.c)

find_package(raylib QUIET)
if(NOT raylib_FOUND)
    message(STATUS "Couldn't find raylib, fetching...")

    FetchContent_Declare(
        raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(raylib)
else()
    message(STATUS "Found raylib")
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)

set(CMAKE_REQUIRED_QUIET TRUE)
check_include_file(raygui.h raygui_FOUND)
unset(CMAKE_REQUIRED_QUIET)
if(NOT raygui_FOUND)
    message(STATUS "Couldn't find raygui, fetching...")

    FetchContent_Declare(
        raygui
        GIT_REPOSITORY https://github.com/raysan5/raygui.git
        GIT_TAG 4.0
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(raygui)

    target_include_directories(${PROJECT_NAME} PRIVATE ${raygui_SOURCE_DIR}/src)
else()
    message(STATUS "Found raygui")
endif()

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
