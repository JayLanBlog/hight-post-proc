#pragma once

inline const float* GetFullscreenQuadVertices() {
    static const float vertices[] = {
        // position       // texcoord
        -1.0f, -1.0f,     0.0f, 0.0f,
         1.0f, -1.0f,     1.0f, 0.0f,
        -1.0f,  1.0f,     0.0f, 1.0f,
        -1.0f,  1.0f,     0.0f, 1.0f,
         1.0f, -1.0f,     1.0f, 0.0f,
         1.0f,  1.0f,     1.0f, 1.0f,
    };
    return vertices;
}

inline constexpr int kFullscreenQuadVertexCount = 6;
