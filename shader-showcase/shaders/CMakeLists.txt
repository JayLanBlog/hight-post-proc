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
