cmake_minimum_required(VERSION 3.16)
project(InstallZone VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# JPL style warning flags
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()

# Find OpenGL & SDL2
find_package(OpenGL REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2 REQUIRED sdl2)

# Include directories
set(IZ_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui/backends
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/duktape
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/nlohmann
    ${OPENGL_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIRS}
)

include_directories(${IZ_INCLUDE_DIRS})

# Third-party sources
set(THIRD_PARTY_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/duktape/duktape.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui/backends/imgui_impl_sdl2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/imgui/backends/imgui_impl_opengl2.cpp
)

# Core library sources
set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/core/iz_i18n.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/iz_project.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/iz_js_engine.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/iz_archive.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/iz_system.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/iz_ui_common.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/iz_cmake_scanner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/core/iz_audio.cpp
    ${THIRD_PARTY_SOURCES}
)

add_library(installzone_core STATIC ${CORE_SOURCES})
target_link_libraries(installzone_core
    ${OPENGL_LIBRARIES}
    ${SDL2_LIBRARIES}
    pthread
    dl
    m
)

# Application 1: InstallZone Studio
add_executable(installzone-studio ${CMAKE_CURRENT_SOURCE_DIR}/studio/main.cpp)
target_link_libraries(installzone-studio installzone_core)

# Application 2: InstallZone Base Runner
add_executable(installzone-runner ${CMAKE_CURRENT_SOURCE_DIR}/runner/main.cpp)
target_link_libraries(installzone-runner installzone_core)

# Installation targets
install(TARGETS installzone-studio installzone-runner
    RUNTIME DESTINATION bin
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../assets/desktop/installzone-studio.desktop
    DESTINATION share/applications
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../assets/icons/installzone-studio.svg
    DESTINATION share/icons/hicolor/scalable/apps
)

install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../assets/icons/installzone-studio.svg
    DESTINATION share/pixmaps
)

install(FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/banner.jpg
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/banner.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/banner_studio.png
    ${CMAKE_CURRENT_SOURCE_DIR}/../assets/chiptune.wav
    DESTINATION share/installzone/assets
)

install(TARGETS installzone-runner
    RUNTIME DESTINATION share/installzone/templates
)
