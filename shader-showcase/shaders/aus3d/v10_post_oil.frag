#version 460
// 油画特效：邻域颜色统计
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    float radius = P0 * 5.0 + 1.0;
    vec2 texelSize = 1.0 / vec2(textureSize(uInputTex, 0));
    
    vec4 avgColor = vec4(0.0);
    float count = 0.0;
    for (float dy = -radius; dy <= radius; dy += 1.0) {
        for (float dx = -radius; dx <= radius; dx += 1.0) {
            vec2 offset = vec2(dx, dy) * texelSize;
            avgColor += texture(uInputTex, vUV + offset);
            count += 1.0;
        }
    }
    outColor = avgColor / count;
}