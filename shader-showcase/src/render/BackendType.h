#pragma once
#include <cstdint>

enum class BackendType { OpenGL, Vulkan };
enum class TextureFormat { RGBA8, RGBA32F, R8 };

struct ShaderHandle { uint32_t id = 0; };
struct TextureHandle { uint32_t id = 0; };
struct PipelineHandle { uint32_t id = 0; };

constexpr ShaderHandle INVALID_SHADER  = { 0xFFFFFFFF };
constexpr TextureHandle INVALID_TEXTURE = { 0xFFFFFFFF };
