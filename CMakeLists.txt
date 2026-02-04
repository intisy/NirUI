cmake_minimum_required(VERSION 3.20)
project(NirUI VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-specific settings
if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
    set(CMAKE_WIN32_EXECUTABLE ON)
endif()

# Fetch dependencies
include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    nanosvg
    GIT_REPOSITORY https://github.com/memononen/nanosvg.git
    GIT_TAG master
)
FetchContent_MakeAvailable(nanosvg)

# ImGui source files
set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)

# Source files
set(SOURCES
    src/main.cpp
    src/core/nircmd_commands.cpp
    src/core/nircmd_manager.cpp
    src/core/app_groups.cpp
    src/cli/cli_parser.cpp
    src/ui/ui_app.cpp
    src/ui/svg_icons.cpp
    src/utils/http_downloader.cpp
)

# Windows resource file (for icon)
if(WIN32)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets/icon.ico")
        set(RESOURCES src/resources.rc)
    endif()
endif()

# Header files
set(HEADERS
    src/core/nircmd_commands.h
    src/core/nircmd_manager.h
    src/core/app_groups.h
    src/cli/cli_parser.h
    src/ui/ui_app.h
    src/ui/svg_icons.h
    src/utils/http_downloader.h
)

# Create executable
add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS} ${IMGUI_SOURCES} ${RESOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${nanosvg_SOURCE_DIR}/src
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    d3d11
    dxgi
    d3dcompiler
    dwmapi
    wininet
    urlmon
    shell32
    ole32
    uuid
)

# Console version for CLI
add_executable(${PROJECT_NAME}_cli
    src/main.cpp
    src/core/nircmd_commands.cpp
    src/core/nircmd_manager.cpp
    src/core/app_groups.cpp
    src/cli/cli_parser.cpp
    src/ui/ui_app.cpp
    src/ui/svg_icons.cpp
    src/utils/http_downloader.cpp
    ${IMGUI_SOURCES}
)

target_include_directories(${PROJECT_NAME}_cli PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${nanosvg_SOURCE_DIR}/src
)

target_link_libraries(${PROJECT_NAME}_cli PRIVATE
    d3d11
    dxgi
    d3dcompiler
    dwmapi
    wininet
    urlmon
    shell32
    ole32
    uuid
)

target_compile_definitions(${PROJECT_NAME}_cli PRIVATE NIRUI_CLI_MODE)

# Copy NirCmd if exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/nircmd.exe")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/nircmd.exe"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()
