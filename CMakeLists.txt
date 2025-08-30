cmake_minimum_required(VERSION 3.10)

project(demo C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

file(GLOB_RECURSE SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.c")
set(RESOURCE_FILES "${CMAKE_SOURCE_DIR}/src/ui/app.rc")

add_executable(${PROJECT_NAME} ${SRC_FILES} ${RESOURCE_FILES})

# Allow MSVC use main as entry point instead of WinMain
if (MSVC)
    set_property(TARGET ${PROJECT_NAME} PROPERTY LINK_FLAGS "/ENTRY:\"mainCRTStartup\"")
endif()

set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE TRUE)

target_link_libraries(${PROJECT_NAME} PRIVATE user32 dwmapi)
