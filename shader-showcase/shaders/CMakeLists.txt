find_program(GLSLANG_VALIDATOR glslangValidator
    HINTS "$ENV{VULKAN_SDK}/Bin"
    DOC "glslangValidator for SPIR-V compilation")

if(NOT GLSLANG_VALIDATOR)
    message(WARNING "glslangValidator not found - shaders will not compile")
    return()
endif()

message(STATUS "glslangValidator: ${GLSLANG_VALIDATOR}")

set(SHADER_OUTPUT_DIR "${CMAKE_BINARY_DIR}/shaders")

# -------------------------------------------------------------------
# Helper function: compile one .frag -> .spv
# -------------------------------------------------------------------
function(compile_frag EFFECT_NAME)
    set(FRAG_SRC "${CMAKE_CURRENT_SOURCE_DIR}/effects/${EFFECT_NAME}/${EFFECT_NAME}.frag")
    set(FRAG_SPV "${SHADER_OUTPUT_DIR}/effects/${EFFECT_NAME}/${EFFECT_NAME}.frag.spv")
    set(EFFECT_JSON_SRC "${CMAKE_CURRENT_SOURCE_DIR}/effects/${EFFECT_NAME}/effect.json")
    set(EFFECT_JSON_DST "${SHADER_OUTPUT_DIR}/effects/${EFFECT_NAME}/effect.json")

    # Copy effect.json if it exists
    if(EXISTS "${EFFECT_JSON_SRC}")
        add_custom_command(
            OUTPUT ${FRAG_SPV} ${EFFECT_JSON_DST}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_OUTPUT_DIR}/effects/${EFFECT_NAME}"
            COMMAND ${GLSLANG_VALIDATOR} -V "${FRAG_SRC}" -o "${FRAG_SPV}"
            COMMAND ${CMAKE_COMMAND} -E copy_if_different "${EFFECT_JSON_SRC}" "${EFFECT_JSON_DST}"
            DEPENDS ${FRAG_SRC} ${EFFECT_JSON_SRC}
            COMMENT "Compiling ${EFFECT_NAME}.frag -> SPIR-V + copying effect.json"
        )
        set(SHADER_SPV_LIST ${SHADER_SPV_LIST} ${FRAG_SPV} ${EFFECT_JSON_DST} PARENT_SCOPE)
    else()
        add_custom_command(
            OUTPUT ${FRAG_SPV}
            COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_OUTPUT_DIR}/effects/${EFFECT_NAME}"
            COMMAND ${GLSLANG_VALIDATOR} -V "${FRAG_SRC}" -o "${FRAG_SPV}"
            DEPENDS ${FRAG_SRC}
            COMMENT "Compiling ${EFFECT_NAME}.frag -> SPIR-V"
        )
        set(SHADER_SPV_LIST ${SHADER_SPV_LIST} ${FRAG_SPV} PARENT_SCOPE)
    endif()
endfunction()

# -------------------------------------------------------------------
# Vertex shaders (OpenGL and Vulkan variants)
# -------------------------------------------------------------------
set(VERT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/common/fullscreen.vert")
set(VERT_SPV "${SHADER_OUTPUT_DIR}/common/fullscreen.vert.spv")
add_custom_command(
    OUTPUT ${VERT_SPV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_OUTPUT_DIR}/common"
    COMMAND ${GLSLANG_VALIDATOR} -V "${VERT_SRC}" -o "${VERT_SPV}"
    DEPENDS ${VERT_SRC}
    COMMENT "Compiling fullscreen.vert -> SPIR-V"
)

# Vulkan-specific vertex shader (uses VertexIndex instead of vertex input)
set(VERT_VK_SRC "${CMAKE_CURRENT_SOURCE_DIR}/common/fullscreen_vk.vert")
set(VERT_VK_SPV "${SHADER_OUTPUT_DIR}/common/fullscreen_vk.vert.spv")
add_custom_command(
    OUTPUT ${VERT_VK_SPV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_OUTPUT_DIR}/common"
    COMMAND ${GLSLANG_VALIDATOR} -V "${VERT_VK_SRC}" -o "${VERT_VK_SPV}"
    DEPENDS ${VERT_VK_SRC}
    COMMENT "Compiling fullscreen_vk.vert -> SPIR-V"
)

# -------------------------------------------------------------------
# All fragment shaders (17 effects)
# -------------------------------------------------------------------
compile_frag(simple_test)
compile_frag(bloom)
compile_frag(blur)
compile_frag(sharpen)
compile_frag(edge_detect)
compile_frag(emboss)
compile_frag(pixelate)
compile_frag(vignette)
compile_frag(chromatic)
compile_frag(color_grade)
compile_frag(noise)
compile_frag(kaleidoscope)
compile_frag(glitch)
compile_frag(toon)
compile_frag(vhs)
compile_frag(crt)
compile_frag(water_ripple)
compile_frag(lens_distort)

# === XPL Glitch Effects (17) ===
compile_frag(xpl_glitch_screen_jump)
compile_frag(xpl_glitch_screen_shake)
compile_frag(xpl_glitch_scan_line_jitter)
compile_frag(xpl_glitch_rgb_split_v4)
compile_frag(xpl_glitch_rgb_split_v2)
compile_frag(xpl_glitch_analog_noise)
compile_frag(xpl_glitch_image_block_v3)
compile_frag(xpl_glitch_image_block_v4)
compile_frag(xpl_glitch_tile_jitter)
compile_frag(xpl_glitch_rgb_split_v3)
compile_frag(xpl_glitch_digital_stripe)
compile_frag(xpl_glitch_image_block_v2)
compile_frag(xpl_glitch_image_block_v1)
compile_frag(xpl_glitch_rgb_split_v1)
compile_frag(xpl_glitch_rgb_split_v5)
compile_frag(xpl_glitch_line_block)
compile_frag(xpl_glitch_wave_jitter)
compile_frag(xpl_blur_bokeh)
compile_frag(xpl_blur_box)
compile_frag(xpl_blur_directional)
compile_frag(xpl_blur_dual_box)
compile_frag(xpl_blur_dual_gaussian)
compile_frag(xpl_blur_dual_kawase)
compile_frag(xpl_blur_dual_tent)
compile_frag(xpl_blur_gaussian)
compile_frag(xpl_blur_grainy)
compile_frag(xpl_blur_iris)
compile_frag(xpl_blur_iris_v2)
compile_frag(xpl_blur_kawase)
compile_frag(xpl_blur_radial)
compile_frag(xpl_blur_radial_v2)
compile_frag(xpl_blur_tent)
compile_frag(xpl_blur_tilt_shift)
compile_frag(xpl_blur_tilt_shift_v2)
compile_frag(xpl_color_bleach_bypass)
compile_frag(xpl_color_brightness)
compile_frag(xpl_color_contrast)
compile_frag(xpl_color_contrast_v2)
compile_frag(xpl_color_contrast_v3)
compile_frag(xpl_color_hue)
compile_frag(xpl_color_lens_filter)
compile_frag(xpl_color_saturation)
compile_frag(xpl_color_technicolor)
compile_frag(xpl_color_tint)
compile_frag(xpl_color_white_balance)
compile_frag(xpl_color_replace)
compile_frag(xpl_color_replace_v2)
compile_frag(xpl_edge_roberts)
compile_frag(xpl_edge_roberts_neon)
compile_frag(xpl_edge_roberts_neon_v2)
compile_frag(xpl_edge_scharr)
compile_frag(xpl_edge_scharr_neon)
compile_frag(xpl_edge_scharr_neon_v2)
compile_frag(xpl_edge_sobel)
compile_frag(xpl_edge_sobel_neon)
compile_frag(xpl_edge_sobel_neon_v2)
compile_frag(xpl_pixel_circle)
compile_frag(xpl_pixel_diamond)
compile_frag(xpl_pixel_hexagon)
compile_frag(xpl_pixel_hexagon_grid)
compile_frag(xpl_pixel_leaf)
compile_frag(xpl_pixel_led)
compile_frag(xpl_pixel_quad)
compile_frag(xpl_pixel_sector)
compile_frag(xpl_pixel_triangle)
compile_frag(xpl_vignette_aurora)
compile_frag(xpl_vignette_old_tv)
compile_frag(xpl_vignette_old_tv_v2)
compile_frag(xpl_vignette_rapid)
compile_frag(xpl_vignette_rapid_v2)
compile_frag(xpl_sharpen_v1)
compile_frag(xpl_sharpen_v2)
compile_frag(xpl_sharpen_v3)

# -------------------------------------------------------------------
# Aggregate target
# -------------------------------------------------------------------
add_custom_target(CompileShaders ALL
    DEPENDS ${VERT_SPV} ${VERT_VK_SPV} ${SHADER_SPV_LIST}
)

# -------------------------------------------------------------------
# Copy effect.json files to build output (runs every build, idempotent)
# -------------------------------------------------------------------
add_custom_target(CopyEffectJsons ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/effects"
        "${SHADER_OUTPUT_DIR}/effects"
    DEPENDS CompileShaders
    COMMENT "Copying effect.json files to build shaders directory"
)

# -------------------------------------------------------------------
# AUS 3D shaders (ray-marched sphere) — compiled once, not per-effect
# -------------------------------------------------------------------
file(GLOB AUS3D_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/aus3d/*.frag")
set(AUS3D_SPV_LIST)
foreach(AUS3D_SRC ${AUS3D_SOURCES})
    get_filename_component(AUS3D_NAME ${AUS3D_SRC} NAME_WE)
    set(AUS3D_SPV "${SHADER_OUTPUT_DIR}/aus3d/${AUS3D_NAME}.frag.spv")
    add_custom_command(
        OUTPUT ${AUS3D_SPV}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_OUTPUT_DIR}/aus3d"
        COMMAND ${GLSLANG_VALIDATOR} -V "${AUS3D_SRC}" -o "${AUS3D_SPV}"
        DEPENDS ${AUS3D_SRC}
        COMMENT "Compiling ${AUS3D_NAME}.frag -> SPIR-V"
    )
    list(APPEND AUS3D_SPV_LIST ${AUS3D_SPV})
endforeach()

# AUS 3D shared vertex shader
set(AUS3D_VERT_SRC "${CMAKE_CURRENT_SOURCE_DIR}/aus3d/aus3d_shared.vert")
set(AUS3D_VERT_SPV "${SHADER_OUTPUT_DIR}/aus3d/aus3d_shared.vert.spv")
add_custom_command(
    OUTPUT ${AUS3D_VERT_SPV}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_OUTPUT_DIR}/aus3d"
    COMMAND ${GLSLANG_VALIDATOR} -V "${AUS3D_VERT_SRC}" -o "${AUS3D_VERT_SPV}"
    DEPENDS ${AUS3D_VERT_SRC}
    COMMENT "Compiling aus3d_shared.vert -> SPIR-V"
)

add_custom_target(CompileAUS3DShaders ALL
    DEPENDS ${AUS3D_VERT_SPV} ${AUS3D_SPV_LIST}
    COMMENT "Compiling AUS 3D shaders"
)

# -------------------------------------------------------------------
# LiquidGlass shaders
# -------------------------------------------------------------------
file(GLOB LG_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/liquid_glass/*.frag")
set(LG_SPV_LIST)
foreach(LG_FILE ${LG_SOURCES})
    get_filename_component(LG_NAME ${LG_FILE} NAME_WE)
    set(LG_SPV "${SHADER_OUTPUT_DIR}/liquid_glass/${LG_NAME}.frag.spv")
    add_custom_command(
        OUTPUT ${LG_SPV}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${SHADER_OUTPUT_DIR}/liquid_glass"
        COMMAND ${GLSLANG_VALIDATOR} -V "${LG_FILE}" -o "${LG_SPV}"
        DEPENDS ${LG_FILE}
        COMMENT "Compiling ${LG_NAME}.frag -> SPIR-V"
    )
    list(APPEND LG_SPV_LIST ${LG_SPV})
endforeach()

add_custom_target(CompileLiquidGlassShaders ALL
    DEPENDS ${LG_SPV_LIST}
    COMMENT "Compiling LiquidGlass shaders"
)
