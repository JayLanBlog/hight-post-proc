#version 460
// Screen Water Drop — AUS Vol.09
layout(location=0) in vec2 vUV; layout(location=0) out vec4 outColor;
layout(binding=0) uniform sampler2D uInputTex;
layout(std140, binding=1) uniform Params {
    float uParamFloat0,uParamFloat1,uParamFloat2,uParamFloat3,uParamFloat4,uParamFloat5;
    vec2 uResolution; float uTime; float uFrameCount;
};
void main() {
    // Simulate water drop refraction without external normal map
    float scale = clamp(uParamFloat0, 2.0, 20.0);
    float speed = clamp(uParamFloat1, 0.2, 3.0);
    float distort = clamp(uParamFloat2, 0.01, 0.1);
    // Procedural ripple pattern
    float ripple = sin(vUV.x * scale * 3.14159 + uTime * speed) * 
                   cos(vUV.y * scale * 3.14159 + uTime * speed * 0.7) * 0.5 + 0.5;
    float ripple2 = sin(vUV.y * scale * 2.2 + uTime * speed * 1.3) * 
                    cos(vUV.x * scale * 1.8 + uTime * speed * 0.5) * 0.5 + 0.5;
    vec2 offset = vec2(
        (ripple - 0.5) * distort,
        (ripple2 - 0.5) * distort
    );
    outColor = texture(uInputTex, vUV + offset);
}
