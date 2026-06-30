#version 460
// 径向模糊：以中心为原点，多次迭代缩放UV
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    vec2 center = vec2(0.5);
    vec2 uv = vUV - center;
    float intensity = P0 * 0.1;
    float iterations = P1 * 20.0 + 5.0;
    
    vec4 col = vec4(0.0);
    float scale = 1.0;
    for (int i = 0; i < 20; i++) {
        if (float(i) >= iterations) break;
        col += texture(uInputTex, uv * scale + center);
        scale = 1.0 + float(i) * intensity;
    }
    outColor = col / iterations;
}