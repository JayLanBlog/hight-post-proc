// shaders/vfx_fire/particle.frag
// 粒子片段着色器 - 纸屑/烟雾两种类型
// type: 0 = 纸屑 (橙色), 1 = 烟雾 (灰色半透明)
#version 450

layout(binding = 0) uniform sampler2D uInputTex;   // unused (point sprite uses gl_PointCoord)
layout(binding = 1, std140) uniform Params {
    float uP0; float uP1; float uP2; float uP3;  // uP0 = type (0=paper, 1=smoke)
    float uP4; float uP5; float uP6; float uP7;
    float uTime; float uFrameCount; float uPad0; float uPad1;
    mat4 uMVP;
    mat4 uModelView;
    vec4 uLightDir;
    vec4 uLightColor;
    vec4 uEyePos;
    vec4 uParams;
};

layout(location = 0) in float vLife;
layout(location = 0) out vec4 outColor;

void main() {
    // 圆形mask: distance from center (0.5, 0.5)
    vec2 center = gl_PointCoord - 0.5;
    float dist = length(center);
    if (dist > 0.5) discard;

    // alpha falloff from center
    float alpha = 1.0 - smoothstep(0.0, 0.5, dist);

    if (uP0 < 0.5) {
        // 纸屑：橙色→暗红→黑 渐变（基于剩余生命）
        float t = vLife;  // 0=dead, 1=full life
        vec3 color = mix(vec3(1.0, 0.5, 0.0), vec3(0.0, 0.0, 0.0), 1.0 - t);
        outColor = vec4(color * 2.0, alpha * t);
    } else {
        // 烟雾：灰色半透明
        outColor = vec4(0.15, 0.15, 0.15, alpha * 0.3 * vLife);
    }
}