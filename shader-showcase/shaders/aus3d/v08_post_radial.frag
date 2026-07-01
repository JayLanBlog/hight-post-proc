#version 460
// 径向模糊：以中心为原点，多次迭代缩放UV
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float P0,P1,P2,P3,P4,P5; vec2 uRes; float uTime,uFC; mat4 m0,m1;
    vec3 uLightDir; float _p0; vec3 uLightColor; float _p1; vec3 uEyePos; float _p2;
};

void main() {
    vec2 center = vec2(P2, P3);
    vec2 uv = vUV - center;
    float intensity = P0 * 0.085;
    int iterations = int(P1);
    
    vec4 col = vec4(0.0);
    float scale = 1.0;
    for (int j = 1; j < 32; j++) {
        if (float(j) >= float(iterations)) break;
        col += texture(uInputTex, uv * scale + center);
        scale = 1.0 + float(j) * intensity;
    }
    outColor = col / float(iterations);
}