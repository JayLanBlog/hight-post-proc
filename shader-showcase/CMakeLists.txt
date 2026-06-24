cmake_minimum_required(VERSION 3.20)
project(ShaderShowcase VERSION 1.0.0 LANGUAGES CXX)

# ---------------------------------------------------------------------------
# MSVC: force UTF-8 source/execution charset
# ---------------------------------------------------------------------------
if(MSVC)
    add_compile_options(/utf-8)
endif()

# ---------------------------------------------------------------------------
# C++ standard
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Output directories
# ---------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(USE_OPENGL "Build with OpenGL backend" ON)
option(USE_VULKAN "Build with Vulkan backend" ON)

# ---------------------------------------------------------------------------
# CMake module path
# ---------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ---------------------------------------------------------------------------
# GLFW — via FetchContent
# ---------------------------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY  https://github.com/glfw/glfw.git
    GIT_TAG         3.4
    GIT_SHALLOW     TRUE
)

# Prevent glfw from building docs / examples / tests / installing
set(GLFW_BUILD_DOCS     OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL        OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)

# ---------------------------------------------------------------------------
# OpenGL
# ---------------------------------------------------------------------------
if(USE_OPENGL)
    find_package(OpenGL REQUIRED)
endif()

# ---------------------------------------------------------------------------
# Vulkan (optional)
# ---------------------------------------------------------------------------
if(USE_VULKAN)
    # Try common Vulkan SDK locations on Windows first
    if(WIN32 AND DEFINED ENV{VULKAN_SDK})
        set(VULKAN_SDK "$ENV{VULKAN_SDK}")
    endif()

    if(VULKAN_SDK)
        message(STATUS "Vulkan SDK found: ${VULKAN_SDK}")
        set(Vulkan_INCLUDE_DIR "${VULKAN_SDK}/Include")
        if(WIN32)
            set(Vulkan_LIBRARY "${VULKAN_SDK}/Lib/vulkan-1.lib")
        else()
            find_library(Vulkan_LIBRARY vulkan HINTS "${VULKAN_SDK}/lib")
        endif()
    else()
        find_package(Vulkan)
    endif()

    if(Vulkan_INCLUDE_DIR AND Vulkan_LIBRARY)
        set(VULKAN_FOUND TRUE CACHE BOOL "Vulkan found" FORCE)
        message(STATUS "Vulkan include: ${Vulkan_INCLUDE_DIR}")
        message(STATUS "Vulkan library: ${Vulkan_LIBRARY}")
    else()
        message(WARNING "Vulkan not found — building without Vulkan support")
        set(VULKAN_FOUND FALSE CACHE BOOL "Vulkan found" FORCE)
    endif()
else()
    set(VULKAN_FOUND FALSE CACHE BOOL "Vulkan found" FORCE)
endif()

# ---------------------------------------------------------------------------
# stb_image — header-only
# ---------------------------------------------------------------------------
set(STB_IMAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/stb")
if(NOT EXISTS "${STB_IMAGE_DIR}/stb_image.h")
    message(FATAL_ERROR "stb_image.h not found at ${STB_IMAGE_DIR}. "
                        "Download it from https://github.com/nothings/stb")
endif()

add_library(stb_image INTERFACE)
target_include_directories(stb_image INTERFACE ${STB_IMAGE_DIR})
# STB_IMAGE_IMPLEMENTATION is now defined in src/stb_impl.cpp

# ---------------------------------------------------------------------------
# ImGui — add_subdirectory
# ---------------------------------------------------------------------------
set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")
if(NOT EXISTS "${IMGUI_DIR}/imgui.h")
    message(FATAL_ERROR "ImGui not found at ${IMGUI_DIR}. "
                        "Clone https://github.com/ocornut/imgui (docking branch)")
endif()

# Pass Vulkan variables to ImGui subdirectory
if(VULKAN_FOUND)
    set(IMGUI_VULKAN_FOUND TRUE)
    set(IMGUI_Vulkan_INCLUDE_DIR ${Vulkan_INCLUDE_DIR})
    set(IMGUI_Vulkan_LIBRARY ${Vulkan_LIBRARY})
endif()

add_subdirectory(external/imgui)

# ---------------------------------------------------------------------------
# Shaders subdirectory
# ---------------------------------------------------------------------------
add_subdirectory(shaders)

# ---------------------------------------------------------------------------
# glslangValidator (optional helper)
# ---------------------------------------------------------------------------
find_program(GLSLANG_VALIDATOR glslangValidator
    HINTS
        "$ENV{VULKAN_SDK}/Bin"
)
if(GLSLANG_VALIDATOR)
    message(STATUS "glslangValidator found: ${GLSLANG_VALIDATOR}")
else()
    message(STATUS "glslangValidator not found (GLSL -> SPIR-V compilation disabled)")
endif()

# ---------------------------------------------------------------------------
# Main executable
# ---------------------------------------------------------------------------
set(SRC_FILES
    src/main.cpp
    src/stb_impl.cpp
    src/app/Application.cpp
    src/app/Application.h
    src/app/CoverFlowScene.cpp
    src/app/CoverFlowScene.h
    src/app/CoverFlowState.h
    src/app/EffectDetailScene.cpp
    src/app/EffectDetailScene.h
    src/app/LanguageManager.cpp
    src/app/LanguageManager.h
    src/app/Scene.h
    src/app/PageManager.cpp
    src/app/PageManager.h
    src/app/SceneRegistry.cpp
    src/app/SceneRegistry.h
    src/app/SceneGalleryScene.cpp
    src/app/SceneGalleryScene.h
    src/render/BackendType.h
    src/render/FullscreenQuad.cpp
    src/render/FullscreenQuad.h
    src/render/IRenderBackend.h
    src/render/OpenGLBackend.cpp
    src/render/OpenGLBackend.h
    src/render/VulkanBackend.cpp
    src/render/VulkanBackend.h
    src/render/gl_core_46.cpp
    src/render/gl_core_46.h
    src/shader/EffectMetadata.cpp
    src/shader/EffectMetadata.h
    src/shader/ShaderLoader.cpp
    src/shader/ShaderLoader.h
    src/ui/DebugPanel.cpp
    src/ui/DebugPanel.h
    src/ui/PerformancePanel.cpp
    src/ui/PerformancePanel.h
    src/input/ScreenCapture.h
    src/input/ScreenCapture.cpp
    src/input/VideoPlayer.h
    src/input/VideoPlayer.cpp
    external/imgui/backends/imgui_impl_vulkan.cpp
)

add_executable(ShaderShowcase ${SRC_FILES})

target_include_directories(ShaderShowcase PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_link_libraries(ShaderShowcase PRIVATE
    glfw
    OpenGL::GL
    stb_image
    imgui
)

if(VULKAN_FOUND)
    target_include_directories(ShaderShowcase PRIVATE ${Vulkan_INCLUDE_DIR})
    target_link_libraries(ShaderShowcase PRIVATE ${Vulkan_LIBRARY})
    target_compile_definitions(ShaderShowcase PRIVATE SHADER_SHOWCASE_HAS_VULKAN USE_VULKAN_BACKEND)
endif()

# ---------------------------------------------------------------------------
# Platform defines
# ---------------------------------------------------------------------------
# Backend type defines
target_compile_definitions(ShaderShowcase PRIVATE USE_OPENGL_BACKEND)

if(WIN32)
    target_compile_definitions(ShaderShowcase PRIVATE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )
    target_link_libraries(ShaderShowcase PRIVATE d3d11 dxgi)
endif()

# ---------------------------------------------------------------------------
# Test executable: automated shader validation
# ---------------------------------------------------------------------------
add_executable(test_shaders EXCLUDE_FROM_ALL
    tools/test_shaders.cpp
    src/render/OpenGLBackend.cpp
    src/render/gl_core_46.cpp
    src/render/FullscreenQuad.cpp
    src/shader/ShaderLoader.cpp
    src/shader/EffectMetadata.cpp
)
target_include_directories(test_shaders PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(test_shaders PRIVATE glfw OpenGL::GL stb_image imgui)
if(WIN32)
    target_compile_definitions(test_shaders PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN USE_OPENGL_BACKEND)
endif()

# ---------------------------------------------------------------------------
# Copy DLLs next to executable on Windows
# ---------------------------------------------------------------------------
if(WIN32)
    add_custom_command(TARGET ShaderShowcase POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:glfw>
            $<TARGET_FILE_DIR:ShaderShowcase>
    )
endif()
