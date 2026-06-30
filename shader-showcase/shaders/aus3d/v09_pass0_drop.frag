#version 460
// Pass0: 采样水滴纹理 → 计算UV偏移量
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex; // 未使用
layout(binding=2) uniform sampler2D uAuxTex;    // 水滴纹理
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    float speed = P0 * 0.3;
    float distortion = P1;
    
    // 三层不同缩放/速度的水滴采样
    vec2 uv = vUV;
    vec3 t1 = texture(uAuxTex, vec2(uv.x * 1.15, uv.y * 1.1 + uTime * speed * 0.15)).rgb;
    vec3 t2 = texture(uAuxTex, vec2(uv.x * 1.25 - 0.1, uv.y * 1.2 + uTime * speed * 0.2)).rgb;
    vec3 t3 = texture(uAuxTex, vec2(uv.x * 0.9, uv.y * 1.25 + uTime * speed * 0.032)).rgb;
    
    // 合成UV偏移 (R/G通道作为dx/dy)
    float dx = (t1.r + t2.r + t3.r) / 3.0 - 0.5;
    float dy = (t1.g + t2.g + t3.g) / 3.0 - 0.5;
    
    // 输出偏移量 (编码为颜色)
    outColor = vec4(dx * distortion, dy * distortion, 0.0, 1.0);
}