#version 460
// LiquidGlass: 13-tap 可分离高斯模糊 (σ≈1.0)
// 与 Blur.glsl blur13() 完全一致
// 水平: P0=radius, P1=0; 垂直: P0=0, P1=radius

layout(location = 0) in vec2 vUV;
layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D uInputTex;
layout(std140, binding = 1) uniform Params {
    float P0;       // 方向 X (已乘以 u_radius)
    float P1;       // 方向 Y (已乘以 u_radius)
    float _pad0;    // P2 占位 (C++ UBO offset 8)
    float _pad1;    // P3 占位 (C++ UBO offset 12)
    float _pad2;    // P4 占位 (C++ UBO offset 16)
    float _pad3;    // P5 占位 (C++ UBO offset 20)
    vec2 uRes;      // RT 分辨率 (C++ offset 24)
    float uTime;
    float uFC;
};

vec4 blur13(sampler2D image, vec2 uv, vec2 resolution, vec2 direction) {
    vec4 color = vec4(0.0);
    vec2 off1 = vec2(1.411764705882353) * direction;
    vec2 off2 = vec2(3.2941176470588234) * direction;
    vec2 off3 = vec2(5.176470588235294) * direction;
    color += texture(image, uv) * 0.1964825501511404;
    color += texture(image, uv + (off1 / resolution)) * 0.2969069646728344;
    color += texture(image, uv - (off1 / resolution)) * 0.2969069646728344;
    color += texture(image, uv + (off2 / resolution)) * 0.09447039785044732;
    color += texture(image, uv - (off2 / resolution)) * 0.09447039785044732;
    color += texture(image, uv + (off3 / resolution)) * 0.010381362401148057;
    color += texture(image, uv - (off3 / resolution)) * 0.010381362401148057;
    return color;
}

void main() {
    outColor = blur13(uInputTex, vUV, uRes, vec2(P0, P1));
}