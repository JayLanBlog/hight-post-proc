// shaders/vfx_fire/particle.vert
// 粒子顶点着色器 - 点精灵渲染
// 顶点格式(32字节): pos(3f) + size/life/rotation(3f) + padding(2f)
// UBO布局: 与VulkanBackend 224字节布局对齐
#version 450

layout(location = 0) in vec3 aPos;       // 粒子世界位置
layout(location = 1) in vec3 aParticle;  // x=size, y=life, z=rotation
layout(location = 2) in vec2 aPad;       // 未使用

layout(binding = 1, std140) uniform Params {
    float uP0; float uP1; float uP2; float uP3;
    float uP4; float uP5; float uP6; float uP7;
    float uTime; float uFrameCount; float uPad0; float uPad1;
    mat4 uMVP;         // offset 48
    mat4 uModelView;   // offset 112
    vec4 uLightDir;    // offset 176
    vec4 uLightColor;  // offset 192
    vec4 uEyePos;      // offset 208
    vec4 uParams;      // offset 224
};

layout(location = 0) out vec3 vData;   // x=size, y=lifeRatio, z=rotation
layout(location = 1) out vec2 vUV;

void main() {
    vec4 clipPos = uMVP * vec4(aPos, 1.0);
    gl_Position = clipPos;
    gl_PointSize = aParticle.x * 50.0 / -clipPos.z;  // 透视缩放
    vData = aParticle;  // size, life, rotation
    vUV = vec2(0.5, 0.5);
}